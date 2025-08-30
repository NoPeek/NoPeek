// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

from fastapi import FastAPI, UploadFile, File, Depends, Body
from sqlalchemy import create_engine, Column, String, Text, DateTime, Integer, ForeignKey
from sqlalchemy.orm import sessionmaker, declarative_base, relationship, Session
from datetime import datetime
import uuid
import os
from PIL import Image
import io
from dotenv import load_dotenv
import subprocess
import time
import base64
import json
import cv2
import numpy as np

# -------------------- 初始化 --------------------
from starlette.responses import JSONResponse

load_dotenv()
app = FastAPI()

UPLOAD_DIR = "uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# -------------------- 数据库配置 --------------------
SQLALCHEMY_DATABASE_URL = (
    f"mysql+pymysql://{os.getenv('DB_USER')}:{os.getenv('DB_PASSWORD')}"
    f"@{os.getenv('DB_HOST')}:{os.getenv('DB_PORT')}/{os.getenv('DB_NAME')}"
)

engine = create_engine(SQLALCHEMY_DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# -------------------- 数据库模型 --------------------
class Post(Base):
    __tablename__ = "posts"

    id = Column(Integer, primary_key=True, index=True)
    post_id = Column(String(36), unique=True, index=True)
    caption = Column(Text, nullable=True)
    user_id = Column(String(100), nullable=False)
    created_at = Column(DateTime, default=datetime.utcnow)

    images = relationship("PostImage", back_populates="post", cascade="all, delete-orphan")

class PostImage(Base):
    __tablename__ = "post_images"

    id = Column(Integer, primary_key=True, index=True)
    image_id = Column(String(36), unique=True, index=True)
    post_id = Column(String(36), ForeignKey("posts.post_id"))
    image_url = Column(String(500), nullable=False)
    file_size = Column(Integer, nullable=False)
    mime_type = Column(String(100), nullable=False)
    width = Column(Integer, nullable=False)
    height = Column(Integer, nullable=False)
    sort_order = Column(Integer, default=0)

    post = relationship("Post", back_populates="images")

Base.metadata.create_all(bind=engine)

# -------------------- 工具函数 --------------------
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def generate_unique_filename(original_filename: str) -> str:
    ext = os.path.splitext(original_filename)[1].lower()
    allowed_extensions = {".jpg", ".jpeg", ".png", ".gif", ".webp", ".bmp"}
    if ext not in allowed_extensions:
        ext = ".jpg"
    return f"{uuid.uuid4().hex}{ext}"

def base64_to_image(base64_string: str) -> np.ndarray:
    # 移除可能的数据URL前缀
    if ',' in base64_string:
        base64_string = base64_string.split(',')[1]
    
    # 解码base64字符串
    image_data = base64.b64decode(base64_string)
    
    # 将字节数据转换为numpy数组
    nparr = np.frombuffer(image_data, np.uint8)
    
    # 解码图像
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    return img

def image_to_base64(image: np.ndarray) -> str:
    # 编码图像为JPEG格式
    _, buffer = cv2.imencode('.jpg', image)
    
    # 转换为base64字符串
    base64_string = base64.b64encode(buffer).decode('utf-8')
    
    return f"data:image/jpeg;base64,{base64_string}"

def run_detection(image_path: str, detection_type: str) -> list:
    """运行检测并返回结果"""
    try:
        # 构建命令
        cmd = ["python", "detect.py", "-i", image_path, "-t", detection_type]
        
        # 执行命令
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=os.path.dirname(os.path.abspath(__file__)))
        
        if result.returncode != 0:
            print(f"检测错误 ({detection_type}): {result.stderr}")
            return []
        
        # 读取生成的JSON文件
        json_path = os.path.splitext(image_path)[0] + ".json"
        
        if not os.path.exists(json_path):
            print(f"JSON文件未找到: {json_path}")
            return []
        
        with open(json_path, 'r') as f:
            detections = json.load(f)
        
        # 为每个检测结果添加类型信息
        for detection in detections:
            detection['type'] = detection_type
        
        return detections
    except Exception as e:
        print(f"运行检测时出错 ({detection_type}): {str(e)}")
        return []


def run_document_detection(image_path: str) -> list:
    """运行文档检测并返回结果"""
    try:
        # 确保输入图像存在
        if not os.path.exists(image_path):
            print(f"输入图像不存在: {image_path}")
            return []

        # 构建命令
        json_path = os.path.splitext(image_path)[0] + "_doc.json"
        cmd = ["python", "blur_doc.py", "-i", image_path, "-j", json_path]

        # 执行命令
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=os.path.dirname(os.path.abspath(__file__)))

        if result.returncode != 0:
            print(f"文档检测错误: {result.stderr}")
            # 尝试读取可能已生成的JSON文件
            if os.path.exists(json_path):
                print("但JSON文件已存在，尝试读取...")
            else:
                return []

        # 检查JSON文件是否存在
        if not os.path.exists(json_path):
            print(f"文档JSON文件未找到: {json_path}")
            return []

        with open(json_path, 'r') as f:
            detections = json.load(f)

        # 为每个检测结果添加类型信息
        for detection in detections:
            detection['type'] = "document"

        return detections
    except Exception as e:
        print(f"运行文档检测时出错: {str(e)}")
        return []

def process_image_with_script(input_path: str, output_path: str, json_path: str, script_type: str, detection_type: str = "face") -> bool:
    """使用不同的脚本处理图像"""
    try:
        if script_type == "blur":
            cmd = ["python", "blur.py", "-i", input_path, "-o", output_path, "-j", json_path, "-t", detection_type]
        elif script_type == "sticker":
            cmd = ["python", "sticker.py", "-i", input_path, "-o", output_path, "-j", json_path, "-t", detection_type]
        elif script_type == "cartoon":
            cmd = ["python", "inpaint.py", "-i", input_path, "-o", output_path, "-j", json_path, "-t", detection_type]
        else:
            print(f"未知的处理类型: {script_type}")
            return False
        
        # 执行命令
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=os.path.dirname(os.path.abspath(__file__)))
        
        if result.returncode != 0:
            print(f"{script_type}处理错误: {result.stderr}")
            return False
        
        return os.path.exists(output_path)
    except Exception as e:
        print(f"运行{script_type}处理时出错: {str(e)}")
        return False

def blur_document_regions(image_path: str, output_path: str, detections: list) -> bool:
    """模糊文档区域"""
    try:
        # 读取图像
        img = cv2.imread(image_path)
        if img is None:
            print(f"无法读取图像: {image_path}")
            return False
        
        # 对每个检测区域进行模糊处理
        for detection in detections:
            if detection['type'] == 'document':
                x1, y1, x2, y2 = detection['bbox_xyxy']
                h, w = img.shape[:2]
                
                # 转换为像素坐标
                x1 = int(x1 * w)
                y1 = int(y1 * h)
                x2 = int(x2 * w)
                y2 = int(y2 * h)
                
                # 提取区域并模糊
                region = img[y1:y2, x1:x2]
                blurred_region = cv2.GaussianBlur(region, (51, 51), 0)
                
                # 将模糊后的区域放回原图
                img[y1:y2, x1:x2] = blurred_region
        
        # 保存结果
        cv2.imwrite(output_path, img)
        return True
    except Exception as e:
        print(f"模糊文档区域时出错: {str(e)}")
        return False

# -------------------- 上传接口 --------------------
@app.post("/upload")
async def upload_image(data: dict = Body(...), db: Session = Depends(get_db)):
    try:
        # 1. 获取base64编码的图像数据
        image_base64 = data.get("image_data", "")
        if not image_base64:
            return {"error": "未提供图像数据"}, 400
        
        # 2. 将base64转换为图像并保存
        img = base64_to_image(image_base64)
        filename = generate_unique_filename("uploaded_image.jpg")
        local_input_path = os.path.join(UPLOAD_DIR, filename)
        cv2.imwrite(local_input_path, img)
        
        # 3. 运行人脸和车牌检测
        face_detections = run_detection(local_input_path, "face")
        plate_detections = run_detection(local_input_path, "plate")
        
        # 4. 合并检测结果
        all_detections = face_detections + plate_detections
        
        # 5. 在图像上绘制检测框
        # annotated_image = draw_detections(img, all_detections)
        
        # 6. 将原图和标注图转换为base64
        original_base64 = image_to_base64(img)
        # annotated_base64 = image_to_base64(annotated_image)
        
        # 7. 提取需要返回给前端的数据（只需要bbox_xyxy）
        response_detections = []
        for det in all_detections:
            response_det = {
                "type": det["type"],
                "bbox_xyxy": det["bbox_xyxy"]
            }
            # 如果是人脸，添加gender信息
            if det["type"] == "face" and "attributes" in det and "gender" in det["attributes"]:
                response_det["gender"] = det["attributes"]["gender"]
            response_detections.append(response_det)
        
        # 8. 返回结果
        return JSONResponse({
        #    "original_image": original_base64,
        #    "annotated_image": annotated_base64,
            "detections": response_detections
        })
        
    except Exception as e:
        # 记录错误日志
        print(f"处理失败: {str(e)}")
        return {"error": f"处理失败: {str(e)}"}, 500

# -------------------- 处理图像接口 --------------------
@app.post("/process_image")
async def process_image(data: dict = Body(...)):
    try:
        # 1. 获取参数
        image_base64 = data.get("image_data", "")
        process_type = data.get("type", "")
        
        if not image_base64:
            return {"error": "未提供图像数据"}, 400
        
        # 2. 将base64转换为图像并保存
        img = base64_to_image(image_base64)
        filename = generate_unique_filename("process_image.jpg")
        local_input_path = os.path.join(UPLOAD_DIR, filename)
        cv2.imwrite(local_input_path, img)
        
        # 3. 运行人脸检测
        face_detections = run_detection(local_input_path, "face")
        
        if not face_detections:
            # 如果没有检测到人脸，直接返回原图
            result_base64 = image_to_base64(img)
            return JSONResponse({
                "processed_image": result_base64,
                "message": "未检测到人脸，返回原图"
            })
        
        # 4. 保存检测结果到JSON文件
        json_filename = os.path.splitext(filename)[0] + ".json"
        json_path = os.path.join(UPLOAD_DIR, json_filename)
        with open(json_path, 'w') as f:
            json.dump(face_detections, f)
        
        # 5. 根据处理类型处理图像
        output_filename = f"processed_{process_type}_{filename}"
        output_path = os.path.join(UPLOAD_DIR, output_filename)
        
        success = process_image_with_script(local_input_path, output_path, json_path, process_type, "face")
        
        if not success:
            return {"error": f"{process_type}处理失败"}, 500
        
        # 6. 读取处理后的图像并转换为base64
        processed_img = cv2.imread(output_path)
        if processed_img is None:
            return {"error": "无法读取处理后的图像"}, 500
        
        result_base64 = image_to_base64(processed_img)
        
        # 7. 清理临时文件
        try:
            os.remove(local_input_path)
            os.remove(json_path)
            os.remove(output_path)
        except:
            pass
        
        # 8. 返回结果
        return JSONResponse({
            "processed_image": result_base64
        })
        
    except Exception as e:
        # 记录错误日志
        print(f"处理失败: {str(e)}")
        return {"error": f"处理失败: {str(e)}"}, 500

# -------------------- 处理文档接口 --------------------
@app.post("/process_doc")
async def process_doc(data: dict = Body(...)):
    try:
        # 1. 获取参数
        image_base64 = data.get("image_data", "")
        process_types = data.get("type", [])

        if not image_base64:
            return {"error": "未提供图像数据"}, 400

        if not process_types:
            return {"error": "未指定处理类型"}, 400

        # 2. 将base64转换为图像并保存
        img = base64_to_image(image_base64)
        if img is None:
            return {"error": "无效的图像数据"}, 400

        filename = generate_unique_filename("process_doc.jpg")
        local_input_path = os.path.join(UPLOAD_DIR, filename)
        success = cv2.imwrite(local_input_path, img)

        if not success:
            return {"error": "保存图像失败"}, 500

        # 3. 运行检测
        detections = []
        current_image_path = local_input_path

        # 处理车牌
        if "license_plate" in process_types:
            plate_detections = run_detection(current_image_path, "plate")
            if plate_detections:
                # 保存检测结果到JSON文件
                plate_json_filename = os.path.splitext(filename)[0] + "_plate.json"
                plate_json_path = os.path.join(UPLOAD_DIR, plate_json_filename)
                with open(plate_json_path, 'w') as f:
                    json.dump(plate_detections, f)

                # 处理车牌
                plate_output_filename = f"processed_plate_{filename}"
                plate_output_path = os.path.join(UPLOAD_DIR, plate_output_filename)

                success = process_image_with_script(current_image_path, plate_output_path, plate_json_path, "blur",
                                                    "plate")

                if success:
                    # 如果之前已经处理过其他内容，删除中间文件
                    if current_image_path != local_input_path:
                        try:
                            os.remove(current_image_path)
                        except:
                            pass
                    current_image_path = plate_output_path
                    detections.extend(plate_detections)
                else:
                    print("车牌处理失败，使用原始图像继续处理文档")

                # 清理临时JSON文件
                try:
                    os.remove(plate_json_path)
                except:
                    pass

        # 处理文档
        if "document_file" in process_types:
            doc_detections = run_document_detection(current_image_path)
            if doc_detections:
                # 处理文档
                doc_output_filename = f"processed_doc_{filename}"
                doc_output_path = os.path.join(UPLOAD_DIR, doc_output_filename)

                success = blur_document_regions(current_image_path, doc_output_path, doc_detections)

                if success:
                    # 如果之前已经处理过其他内容，删除中间文件
                    if current_image_path != local_input_path:
                        try:
                            os.remove(current_image_path)
                        except:
                            pass
                    current_image_path = doc_output_path
                    detections.extend(doc_detections)
                else:
                    print("文档处理失败，使用之前处理的图像")
            else:
                print("未检测到文档，跳过文档处理")

        # 4. 读取处理后的图像并转换为base64
        if not os.path.exists(current_image_path):
            return {"error": "处理后的图像不存在"}, 500

        processed_img = cv2.imread(current_image_path)
        if processed_img is None:
            return {"error": "无法读取处理后的图像"}, 500

        result_base64 = image_to_base64(processed_img)

        # 5. 清理临时文件
        try:
            # 删除原始输入文件
            if os.path.exists(local_input_path):
                os.remove(local_input_path)

            # 删除中间处理文件（如果不是最终输出）
            if current_image_path != local_input_path and current_image_path.endswith("_processed.jpg"):
                os.remove(current_image_path)
        except Exception as e:
            print(f"清理临时文件时出错: {str(e)}")

        # 6. 返回结果
        return JSONResponse({
            "processed_image": result_base64
        })

    except Exception as e:
        # 记录错误日志
        print(f"处理失败: {str(e)}")
        return {"error": f"处理失败: {str(e)}"}, 500

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
