//
//  NativeImageFileManager.swift
//  LynxExplorer
//
//  Created by 李奥 on 2025/8/29.
//

import Foundation
import UIKit

@objcMembers
public final class NativeImageFileManager: NSObject, LynxModule {
  
  @objc public static var name: String {
    return "NativeImageFileManager"
  }
  
  @objc public static var methodLookup: [String : String] {
    return [
      "saveImageFromBase64": NSStringFromSelector(#selector(saveImageFromBase64(_:fileName:completion:))),
      "readImageAsBase64": NSStringFromSelector(#selector(readImageAsBase64(fileName:completion:))),
      "listImages": NSStringFromSelector(#selector(listImages(completion:))),
      "deleteImage": NSStringFromSelector(#selector(deleteImage(fileName:completion:))),
      "getImageDimensionFromBase64": NSStringFromSelector(#selector(getImageDimensionFromBase64(_:completion:)))
    ]
  }
  
  private let fileManager = FileManager.default
  private let imagesDirectory: URL
  
  @objc public init(param: Any) {
    let documentsDirectory = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
    imagesDirectory = documentsDirectory.appendingPathComponent("Images")
      
    if !fileManager.fileExists(atPath: imagesDirectory.path) {
      do {
        try fileManager.createDirectory(at: imagesDirectory, withIntermediateDirectories: true, attributes: nil)
      } catch {
        fatalError("Failed to create images directory at: " + imagesDirectory.absoluteString)
      }
    }
    
    super.init()
  }
  
  @objc public override init() {
    let documentsDirectory = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
    imagesDirectory = documentsDirectory.appendingPathComponent("Images")
    
    if !fileManager.fileExists(atPath: imagesDirectory.path) {
      do {
        try fileManager.createDirectory(at: imagesDirectory, withIntermediateDirectories: true, attributes: nil)
      } catch {
        fatalError("Failed to create images directory at: \(imagesDirectory.absoluteString)")
      }
    }
    
    super.init()
  }
  
  @objc func saveImageFromBase64(
    _ base64String: NSString, fileName: NSString, 
    completion: (NSDictionary) -> Void
  ) -> Void {
    let cleanBase64 = cleanBase64(base64String as String)
    
    guard let imageData = Data(base64Encoded: cleanBase64, options: .ignoreUnknownCharacters) else {
      completion([
        "result": false as NSNumber, 
        "message": "Failed to parse Base64" as NSString
      ] as NSDictionary)
      return
    }
    
    //    let fileExtension = getFileExtension(from: imageData)
    //    let fileNameWithExtension = "\(fileName).\(fileExtension)"

    let image = UIImage(data: imageData)
    let jpgData = image?.jpegData(compressionQuality: 0.8)
    
    // at present, we can assert the filename contains ext
    let fileURL = imagesDirectory.appendingPathComponent(fileName as String)
    NSLog("(NativeImageFileManager)  Saving image to \(fileURL)")
    
    do {
      if jpgData != nil {
        try jpgData?.write(to: fileURL)
      } else {
        try imageData.write(to: fileURL)
      }
      NSLog("(NativeImageFileManager)  Image saved to \(fileURL)")
      completion([
        "result": true as NSNumber, 
        "message": fileURL.absoluteString as NSString
      ] as NSDictionary)
    } catch {
      NSLog("(NativeImageFileManager)  Image failed to save for \(error)")
      completion([
        "result": false as NSNumber, 
        "message": "Failed to save image for \(error)" as NSString
      ] as NSDictionary)
    }
  }
  
  @objc func readImageAsBase64(fileName: NSString, completion: (NSDictionary) -> Void) -> Void {
    let fileURL = imagesDirectory.appendingPathComponent(fileName as String)
      
    do {
      let imageData = try Data(contentsOf: fileURL)
      NSLog("(NativeImageFileManager)  Find image to be converted successfully \(fileURL)")
      
      let image = UIImage(data: imageData)
      NSLog("(NativeImageFileManager)  Image Size: \(String(describing: image?.size))")
      
      let base64 = imageData.base64EncodedString()
      let mimeType = getMimeType(from: imageData)
      
      completion([
        "result": true as NSNumber, 
        "base64OrError": "data:\(mimeType);base64,\(base64)" as NSString,
        "width": NSNumber(value: image?.size.width.native ?? -1),
        "height": NSNumber(value: image?.size.height.native ?? -1)
      ] as NSDictionary)
    } catch {
      NSLog("(NativeImageFileManager)  Failed to read image for: \(error)")
      completion([
        "result": false as NSNumber, 
        "base64OrError": "Failed to read image for: \(error)" as NSString,
        "width": -1 as NSNumber,
        "height": -1 as NSNumber
      ] as NSDictionary)
    }
  }
  
  @objc func listImages(completion: (NSDictionary) -> Void) -> Void {
    do {
      let fileURLs = try fileManager.contentsOfDirectory(at: imagesDirectory, includingPropertiesForKeys: [.creationDateKey])
      
      let imageFiles = fileURLs.filter { url in
        let pathExtension = url.pathExtension.lowercased()
        return ["jpg", "jpeg", "png", "gif", "bmp", "tiff"].contains(pathExtension)
      }
      
      let images = imageFiles.compactMap {
        url in
        do {
          let resourceValues = try url.resourceValues(forKeys: [.creationDateKey])
          NSLog("(NativeImageFileManager)  Find images \(resourceValues)")
          return (fileName: url.lastPathComponent, creationDate: resourceValues.creationDate)
        } catch {
          completion([
            "result": false as NSNumber, 
            "imageFileNames": ["Failed to get image info for: \(error)"] as NSArray
          ] as NSDictionary)
          return nil
        }
      }
      .sorted { $0.creationDate ?? Date() > $1.creationDate ?? Date() }
      .map { $0.fileName }
      
      completion([
        "result": true as NSNumber, 
        "imageFileNames": images as NSArray
      ] as NSDictionary)
    } catch {
      NSLog("(NativeImageFileManager)  Failed to list images for: \(error)")
      completion([
        "result": false as NSNumber, 
        "imageFileNames": ["Failed to list images for: \(error)"] as NSArray
      ] as NSDictionary)
    }
  }
  
  @objc func deleteImage(fileName: NSString, completion: (NSDictionary) -> Void) -> Void {
    let fileURL = imagesDirectory.appendingPathComponent(fileName as String)
    
    do {
      try fileManager.removeItem(at: fileURL)
      NSLog("(NativeImageFileManager)  Image removed \(fileURL)")
      completion([
        "result": true as NSNumber, 
        "errorMessage": "" as NSString
      ] as NSDictionary)
    } catch {
      NSLog("(NativeImageFileManager)  Failed to delete image for: \(error)")
      completion([
        "result": false as NSNumber, 
        "errorMessage": "Failed to delete image for: \(error)" as NSString
      ] as NSDictionary)
    }
  }
  
  @objc func getImageDimensionFromBase64(_ base64String: NSString, completion: (NSDictionary) -> Void) -> Void {
    let cleanBase64 = cleanBase64(base64String as String)
    
    guard let imageData = Data(base64Encoded: cleanBase64, options: .ignoreUnknownCharacters),
          let image = UIImage(data: imageData) else {
      completion([
        "result": false as NSNumber,
        "width": -1 as NSNumber,
        "height": -1 as NSNumber
      ] as NSDictionary)
      return
    }
    
    NSLog("(NativeImageFileManager)  Image Size: \(image.size)")
    completion([
      "result": true as NSNumber,
      "width": NSNumber(value: image.size.width.native),
      "height": NSNumber(value: image.size.height.native)
    ] as NSDictionary)
  }
  
  private func cleanBase64(_ base64String: String) -> String {
    var cleanBase64 = base64String
    if let range = cleanBase64.range(of: "base64,") {
      cleanBase64 = String(cleanBase64[range.upperBound...])
    }
    
    cleanBase64 = cleanBase64.replacingOccurrences(of: "\n", with: "")
    cleanBase64 = cleanBase64.replacingOccurrences(of: "\r", with: "")
    cleanBase64 = cleanBase64.replacingOccurrences(of: " ", with: "")
    cleanBase64 = cleanBase64.replacingOccurrences(of: "\t", with: "")
    
    return cleanBase64
  }
  
  private func getFileExtension(from imageData: Data) -> String {
    guard let imageSource = CGImageSourceCreateWithData(imageData as CFData, nil) else {
      return "jpg"
    }
    
    let type = CGImageSourceGetType(imageSource) as String? ?? "public.jpeg"
    
    switch type {
    case "public.png":
      return "png"
    case "public.jpeg", "public.jpg":
      return "jpg"
    case "public.gif":
      return "gif"
    case "public.tiff":
      return "tiff"
    default:
      return "jpg"
    }
  }
  
  private func getMimeType(from imageData: Data) -> String {
    guard let imageSource = CGImageSourceCreateWithData(imageData as CFData, nil) else {
      return "image/jpeg" // 默认 MIME 类型
    }
    
    let type = CGImageSourceGetType(imageSource) as String? ?? "public.jpeg"
    
    switch type {
    case "public.png":
      return "image/png"
    case "public.jpeg", "public.jpg":
      return "image/jpeg"
    case "public.gif":
      return "image/gif"
    case "public.tiff":
      return "image/tiff"
    default:
      return "image/jpeg"
    }
  }
}
