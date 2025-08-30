// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { useRef, useState } from "@lynx-js/react";
import { useLocation, useNavigate } from "react-router";
import clsx from "clsx";
import ArrowLeftIcon from "../assets/icons/icon_back_solid.png";
import LockIcon from "../assets/icons/icon_lock_outline.png"
import LocationIcon from "../assets/icons/icon_location_outline.png"
import GlobalIcon from "../assets/icons/icon_global_outline.svg"

// 定义图片类型
interface ImageItem {
  id: string;
  src: string;
}

import Avatar1 from "../assets/avatars/avatar-1.jpeg";

export default function PostProceedPage() {
  const location = useLocation();
  const navigate = useNavigate();
  const [currentIndex, setCurrentIndex] = useState(0);
  const [title, setTitle] = useState("");
  const [content, setContent] = useState("");
  const [isPublishing, setIsPublishing] = useState(false);
  const [showPrivacyModal, setShowPrivacyModal] = useState(false);
  const [privacySetting, setPrivacySetting] = useState("Public"); // "Public", "Friends", "Only Me"
  const [isPrivacyClosing, setIsPrivacyClosing] = useState(false);
  const [showToast, setShowToast] = useState(false);
  const [toastMessage, setToastMessage] = useState("");

  // 引用正文字段以便在光标处插入符号
  const contentRef = useRef<any>(null);

  // 显示提示信息
  const showToastMessage = (message: string) => {
    setToastMessage(message);
    setShowToast(true);
    setTimeout(() => {
      setShowToast(false);
    }, 2000);
  };

  const insertAtCursor = (symbol: string) => {
    const textarea = contentRef.current;
    if (!textarea) {
      setContent((prev) => `${prev}${symbol}`);
      return;
    }

    try {
      const start: number = textarea.selectionStart ?? 0;
      const end: number = textarea.selectionEnd ?? start;
      const before = content.slice(0, start);
      const after = content.slice(end);
      const nextValue = `${before}${symbol}${after}`;
      setContent(nextValue);

      setTimeout(() => {
        if (typeof textarea.focus === 'function') textarea.focus();
        if (typeof textarea.setSelectionRange === 'function') {
          const pos = start + symbol.length;
          textarea.setSelectionRange(pos, pos);
        }
      }, 0);
    } catch (_) {
      setContent((prev) => `${prev}${symbol}`);
    }
  };

  const handleInsertHashtag = () => insertAtCursor('#');
  const handleInsertMention = () => insertAtCursor('@');

  // 从导航状态获取图片数据，如果没有则使用默认测试图片
  const images: ImageItem[] =
    location.state?.images || [];

  const handleBack = () => {
    navigate('/gallery'); // 如果没有历史记录，跳转到画廊页面
  };

  const handleSwipe = (direction: "left" | "right") => {
    if (direction === "left") {
      setCurrentIndex((prev) => Math.min(prev + 1, images.length - 1));
    } else {
      setCurrentIndex((prev) => Math.max(prev - 1, 0));
    }
  };

  // 正确的发布函数
  const handlePublish = () => {
    console.log("Publish button clicked");
    console.log("Current title:", title);
    console.log("Current content:", content);
    console.log("Title detail:", typeof title, title.length);
    console.log("Content detail:", typeof content, content.length);
    
    // 验证输入
    if (!title || title.trim().length === 0) {
      console.log("Title is empty");
      showToastMessage("Please input title.");
      return;
    }
    
    if (!content || content.trim().length === 0) {
      console.log("Content is empty");
      showToastMessage("Please input content!");
      return;
    }
    
    console.log("Validation passed - Starting publish process");
    setIsPublishing(true);
    
    try {
      // 创建帖子对象
      const newPost = {
        id: Date.now().toString(),
        title: title.trim(),
        content: content.trim(),
        imageUrl: images[currentIndex].src,
        likes: 0,
        comments: 0,
        bookmarks: 0,
        shares: 0,
        createdAt: Date.now(),
        username: "current_user",
        avatarUrl: Avatar1,
        music: "Original Sound"
    };

      console.log("New post created:", newPost);
      navigate("/browseposts", { state: { newPost } });
      
    } catch (err) {
      console.error("Publish error:", err);
      showToastMessage("Error, please retry.");
    } finally {
      setIsPublishing(false);
    }
  };

  const privacyOptions = [
    {
      value: "Public",
      label: "Public",
      icon: GlobalIcon,
      description: "Anyone can see your post"
    },
    {
      value: "Friends",
      label: "Friends",
      icon: "👥",
      description: "Only your friends can see your post"
    },
    {
      value: "Only Me",
      label: "Only Me",
      icon: LockIcon,
      description: "Only you can see your post"
    }
  ];

  const openPrivacyModal = () => {
    setIsPrivacyClosing(false);
    setShowPrivacyModal(true);
  };

  const closePrivacyModalWithAnimation = () => {
    setIsPrivacyClosing(true);
    setTimeout(() => {
      setShowPrivacyModal(false);
      setIsPrivacyClosing(false);
    }, 220);
  };

  return (
    <view className="w-screen h-screen bg-white flex flex-col">
      {/* Toast 提示 */}
      {showToast && (
        <view className="absolute top-20 left-1/2 transform -translate-x-1/2 bg-black bg-opacity-80 text-white px-4 py-2 rounded-md z-50">
          <text className="text-sm">{toastMessage}</text>
        </view>
      )}

      {/* 顶部导航栏 - 适配iOS安全区域 */}
      <view className="pt-16 h-24 flex items-center justify-between px-4 bg-white z-50">
        <view
          className="w-10 h-10 flex items-center justify-center"
          bindtap={handleBack}
        >
          <image src={ArrowLeftIcon} className="w-7 h-7" />
        </view>
      </view>

      {/* 主要内容区域 */}
      <view className="flex-1 flex flex-col">
        {/* 图片展示区域 - 高度缩小一半 */}
        <view className="h-1/3 relative bg-white">
          <image
            src={images[currentIndex].src}
            className="w-full h-full object-contain"
            mode="aspectFit"
          />

          {images.length > 1 && (
            <>
              <view
                className="absolute left-0 top-0 bottom-0 w-1/3"
                bindtap={() => handleSwipe("right")}
              />
              <view
                className="absolute right-0 top-0 bottom-0 w-1/3"
                bindtap={() => handleSwipe("left")}
              />

              {/* 图片指示器 */}
              <view className="absolute top-4 left-0 right-0 flex justify-center">
                <view className="bg-black bg-opacity-50 rounded-full px-3 py-1 flex gap-1">
                  {images.map((image: ImageItem, index: number) => (
                    <view
                      key={image.id}
                      className={clsx(
                        "w-2 h-2 rounded-full",
                        index === currentIndex ? "bg-white" : "bg-gray-500"
                      )}
                    />
                  ))}
                </view>
              </view>

              {/* 当前图片计数 */}
              <view className="absolute bottom-4 right-4 bg-black bg-opacity-50 rounded-full px-3 py-1">
                <text className="text-white text-sm">
                  {currentIndex + 1} / {images.length}
                </text>
              </view>
            </>
          )}
        </view>

        {/* 输入区域 - 使用Lynx标准写法 */}
        <view className="bg-white p-5 flex flex-col gap-0.5">
            <input
                placeholder="Add Title"
                value={title}
                bindinput={(e: any) => {
                    console.log("Title input event:", e);
                    console.log("Title value:", e.detail?.value);
                    setTitle(e.detail?.value || e.target?.value || '');
                }}
                className="w-full h-12 px-3 py-2 bg-transparent border-0 border-b border-gray-200 outline-none text-lg font-medium"
            />

            <textarea
                placeholder="Describe your ideas..."
                value={content}
                bindinput={(e: any) => {
                    console.log("Content input event:", e);
                    console.log("Content value:", e.detail?.value);
                    setContent(e.detail?.value || e.target?.value || '');
                }}
                ref={contentRef}
                className="w-full h-32 p-3 bg-transparent border-0 outline-none text-base resize-none"
            />
        </view>

        {/* 正文辅助操作按钮 */}
        <view className="bg-white px-5 -mt-2">
          <view className="flex items-center gap-3">
            <view
              className="px-3 py-1 rounded bg-gray-100 text-black"
              bindtap={handleInsertHashtag}
            >
              <text className="text-sm"># Trending</text>
            </view>
            <view
              className="px-3 py-1 rounded bg-gray-100 text-black"
              bindtap={handleInsertMention}
            >
              <text className="text-sm">@ User</text>
            </view>
          </view>
        </view>

        {/* 位置选择和权限设置区域 */}
        <view className="bg-white px-5 py-3 flex flex-col gap-3">
          {/* 位置选择组件 */}
          <view className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
            <view className="flex items-center gap-3">
              <view className="w-6 h-6 bg-transparent rounded-full flex items-center justify-center">
                <image src={LocationIcon} className="w-5 h-5" />
              </view>
              <text className="text-gray-700 font-medium">Location</text>
            </view>
            <view className="flex items-center gap-2">
              <text className="text-gray-500 text-sm">Current Location</text>
              <view className="w-4 h-4 bg-gray-300 rounded-full flex items-center justify-center">
                <text className="text-white text-xs">{'>'}</text>
              </view>
            </view>
          </view>

          {/* 权限设置组件 */}
          <view 
            className="flex items-center justify-between p-3 bg-gray-50 rounded-lg"
            bindtap={openPrivacyModal}
          >
            <view className="flex items-center gap-3">
              <view className="w-6 h-6 bg-white rounded-full flex items-center justify-center">
                <image src={LockIcon} className="w-5 h-5" />
              </view>
              <text className="text-gray-700 font-medium">Privacy</text>
            </view>
            <view className="flex items-center gap-2">
              <text className="text-gray-500 text-sm">{privacySetting}</text>
              <view className="w-4 h-4 bg-gray-300 rounded-full flex items-center justify-center">
                <text className="text-white text-xs">{'>'}</text>
              </view>
            </view>
          </view>
        </view>

        {/* 发布按钮 - 放置在屏幕最底部 */}
        <view className="mt-auto bg-transparent mb-8 p-5 pb-safe-bottom">
          <view
            bindtap={!isPublishing ? handlePublish : undefined}
            className={`h-12 px-5 rounded-2xl flex items-center justify-center text-white bg-[#0369a1] active:bg-[#374151] transition-colors ${isPublishing ? 'opacity-50' : ''}`}
          >
            <text className="text-lg font-semibold text-white">{isPublishing ? "Publishing..." : "Publish"}</text>
          </view>
        </view>

        {/* 权限设置弹窗 */}
        {showPrivacyModal && (
          <view className="fixed inset-0 bg-gray-800 bg-opacity-40 z-50 flex items-end">
            <view className={`bg-white w-full rounded-t-3xl p-6 pb-safe-bottom transform transition-transform duration-200 ${isPrivacyClosing ? 'translate-y-full' : 'translate-y-0'}`}
            >
              <view className="flex items-center justify-between mb-6">
                <text className="text-xl font-semibold text-gray-800">Privacy Setting</text>
                <view 
                  className="w-8 h-8 flex items-center justify-center"
                  bindtap={closePrivacyModalWithAnimation}
                >
                  <text className="text-gray-500 text-2xl">×</text>
                </view>
              </view>
              
              <view className="space-y-4">
                {privacyOptions.map((option) => (
                  <view 
                    key={option.value}
                    className={`flex items-center justify-between p-4 rounded-lg border ${
                      privacySetting === option.value 
                        ? 'border-blue-500 bg-blue-50' 
                        : 'border-gray-200'
                    }`}
                    bindtap={() => {
                      setPrivacySetting(option.value);
                      closePrivacyModalWithAnimation();
                    }}
                  >
                    <view className="flex items-center gap-3">
                      <text className="text-lg">{option.icon}</text>
                      <view>
                        <text className="text-gray-800 font-medium">{option.label}</text>
                        <text className="text-gray-500 text-sm block">{option.description}</text>
                      </view>
                    </view>
                    {privacySetting === option.value && (
                      <view className="w-5 h-5 bg-blue-500 rounded-full flex items-center justify-center">
                        <text className="text-white text-sm">✓</text>
                      </view>
                    )}
                  </view>
                ))}
              </view>
            </view>
          </view>
        )}
      </view>
    </view>
  );
}