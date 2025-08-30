// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// src/pages/tiktokfeed.tsx
import { useState, useEffect, useRef } from "@lynx-js/react";
import { useNavigate, useLocation } from "react-router";
import { Toast, type ToastHandle } from "@/components/toast";

import AddIconOutline from "../assets/icons/icon_add_outline.png";
import HeartIcon from "../assets/icons/icon_heart_outline.png";
import CommentIcon from "../assets/icons/icon_comment_outline.png";
import ShareIcon from "../assets/icons/icon_share_post_outline.png";
import BookmarkIcon from "../assets/icons/icon_book_outline.png";
import BookmarkSolidIcon from "../assets/icons/icon_book_solid.png";
import HeartSolidIcon from "../assets/icons/icon_heart_solid.png";

import Avatar1 from "../assets/avatars/avatar-1.jpeg";
import Avatar2 from "../assets/avatars/avatar-2.jpeg";
import Avatar3 from "../assets/avatars/avatar-3.jpeg";
import Avatar4 from "../assets/avatars/avatar-4.jpeg";

// 导入本地家具图片
import Furniture0 from "../assets/images/1.jpg";
import Furniture1 from "../assets/images/2.jpg";
import Furniture2 from "../assets/images/3.jpg";
import Furniture3 from "../assets/images/4.jpg";

// 定义帖子类型
interface Post {
  id: string;
  title: string;
  content: string;
  imageUrl: string;
  likes: number;
  comments: number;
  bookmarks: number;
  shares: number;
  createdAt: number;
  username: string;
  avatarUrl: string;
  music: string;
}

export default function TikTokFeedPage() {
  const navigate = useNavigate();
  const toastRef = useRef<ToastHandle>(null);
  const location = useLocation();
  const [posts, setPosts] = useState<Post[]>([]);
  const [currentIndex, setCurrentIndex] = useState(0);
  const [likedPosts, setLikedPosts] = useState<Record<string, boolean>>({});
  const [bookmarkedPosts, setBookmarkedPosts] = useState<Record<string, boolean>>({});
  const [animatingLikes, setAnimatingLikes] = useState<Record<string, boolean>>({});
  const [animatingBookmarks, setAnimatingBookmarks] = useState<Record<string, boolean>>({});

  // 初始化帖子数据
  useEffect(() => {
    // 检查是否有新发布的帖子
    const newPost = location.state?.newPost;
    
    // 基础模拟数据
    const basePosts: Post[] = [
      {
        id: "1",
        username: "Lily",
        title: "Modern Living Room",
        content: "This Nordic-style living room setup is amazing! Simple yet elegant. #interiordesign #homedecor",
        imageUrl: Furniture0,
        likes: 12500,
        comments: 342,
        bookmarks: 892,
        shares: 124,
        createdAt: Date.now() - 3600000,
        avatarUrl: Avatar1,
        music: "Relaxing Home Vibes"
      },
      {
        id: "2",
        username: "Vivian",
        title: "Modern Kitchen Design",
        content: "Open kitchens are really the preferred choice for modern families! #kitchendesign #modernhome",
        imageUrl: Furniture1,
        likes: 8900,
        comments: 210,
        bookmarks: 456,
        shares: 89,
        createdAt: Date.now() - 7200000,
        avatarUrl: Avatar2,
        music: "Cooking Inspiration"
      },
      {
        id: "3",
        username: "Leo",
        title: "Cozy Bedroom Setup",
        content: "Soft bedding paired with warm lighting - this is what home feels like. #bedroomdecor #cozyhome",
        imageUrl: Furniture2,
        likes: 15600,
        comments: 421,
        bookmarks: 1203,
        shares: 342,
        createdAt: Date.now() - 10800000,
        avatarUrl: Avatar3,
        music: "Peaceful Nights"
      },
      {
        id: "4",
        username: "minimalist",
        title: "Minimalist Living Room",
        content: "Less is more. This minimalist setup creates such a peaceful atmosphere. #minimalism #simpleliving",
        imageUrl: Furniture3,
        likes: 6700,
        comments: 156,
        bookmarks: 342,
        shares: 78,
        createdAt: Date.now() - 14400000,
        avatarUrl: Avatar4,
        music: "Simple Life"
      }
    ];

    // 如果有新发布的帖子，添加到列表开头
    if (newPost) {
      setPosts([newPost, ...basePosts]);
    } else {
      setPosts(basePosts);
    }
  }, [location.state]);

  useEffect(() => {
    toastRef.current?.show();
    setTimeout(() => toastRef.current?.hide(), 7500);
  }, []);

  // 处理点赞
  const handleLike = (postId: string) => {
    // 触发动画效果
    setAnimatingLikes(prev => ({ ...prev, [postId]: true }));
    setTimeout(() => {
      setAnimatingLikes(prev => ({ ...prev, [postId]: false }));
    }, 300);

    setLikedPosts(prev => ({
      ...prev,
      [postId]: !prev[postId]
    }));
  };

  // 处理收藏
  const handleBookmark = (postId: string) => {
    // 触发动画效果
    setAnimatingBookmarks(prev => ({ ...prev, [postId]: true }));
    setTimeout(() => {
      setAnimatingBookmarks(prev => ({ ...prev, [postId]: false }));
    }, 300);

    setBookmarkedPosts(prev => ({
      ...prev,
      [postId]: !prev[postId]
    }));
  };

  // 页面切换逻辑
//   const switchToIndex = (targetIndex: number) => {
//     const clamped = Math.max(0, Math.min(targetIndex, posts.length - 1));
//     setCurrentIndex(clamped);
//   };

  // 处理滑动切换
//   const handleSwipe = (direction: "up" | "down") => {
//     if (direction === "up" && currentIndex < posts.length - 1) {
//       switchToIndex(currentIndex + 1);
//     } else if (direction === "down" && currentIndex > 0) {
//       switchToIndex(currentIndex - 1);
//     }
//   };

  // 处理创建帖子
  const handleCreatePost = () => {
    navigate("/post");
  };

  // 格式化数字
  const formatNumber = (num: number): string => {
    if (num >= 1000000) {
      return (num / 1000000).toFixed(1) + 'M';
    }
    if (num >= 1000) {
      return (num / 1000).toFixed(1) + 'K';
    }
    return num.toString();
  };

  // 格式化时间
  const formatTime = (timestamp: number) => {
    const now = Date.now();
    const diff = now - timestamp;
    const hours = Math.floor(diff / (1000 * 60 * 60));
    if (hours < 1) return "刚刚";
    if (hours < 24) return `${hours}小时前`;
    const days = Math.floor(hours / 24);
    return `${days}天前`;
  };

  return (
    <view className="w-screen h-screen bg-black flex flex-col">
        <Toast ref={toastRef} style={{
            borderRadius: "16px",
            left: "15vw",
            width: "70vw"
        }} content="Welcome! This is a social media simulation mode. Please tap the '+' button to continue" />
      {/* 主要内容区域 - 占据除底部横条外的所有空间 */}
      <view className="flex-1 relative">
        {posts.length > 0 && (
          <view className="absolute inset-0">
            {/* 图片展示区域 */}
            <view className="w-full h-full relative">
              <image
                src={posts[currentIndex]?.imageUrl}
                className="w-full h-full object-cover"
                mode="aspectFill"
              />
              
              {/* 帖子信息覆盖层 - 继续向下移动 */}
              <view className="absolute bottom-0 left-0 right-0 bg-gradient-to-t from-black/70 to-transparent p-3 pt-8">
                <text className="text-white text-xl font-bold block mb-1">
                  @{posts[currentIndex]?.username}
                </text>
                {/* 限制内容最多显示两行，白色文字 */}
                <text className="text-white text-base block leading-relaxed mb-1 line-clamp-2">
                  {posts[currentIndex]?.content}
                </text>
                <view className="flex items-center gap-2">
                  <image 
                    src={posts[currentIndex]?.avatarUrl} 
                    className="w-5 h-5 rounded-full"
                    mode="aspectFill"
                  />
                  <text className="text-white text-sm">
                    {posts[currentIndex]?.music}
                  </text>
                </view>
              </view>

              {/* 右侧互动按钮 - 距离更近，图标更大，整体向下移动 */}
              <view className="absolute right-4 bottom-28 flex flex-col items-center gap-2">
                {/* 用户头像 */}
                <view className="flex flex-col items-center">
                  <image 
                    src={posts[currentIndex]?.avatarUrl} 
                    className="w-14 h-14 rounded-full border-2 border-white"
                    mode="aspectFill"
                  />
                </view>

                {/* 点赞按钮 - 图标更大 */}
                <view className="flex flex-col items-center">
                  <view
                    className={`w-16 h-16 rounded-full bg-black/30 flex items-center justify-center backdrop-blur-sm cursor-pointer transform transition-all duration-200 ${
                      animatingLikes[posts[currentIndex]?.id] ? 'scale-125' : 'scale-95'
                    }`}
                    bindtap={() => handleLike(posts[currentIndex]?.id)}
                  >
                    <image 
                      src={likedPosts[posts[currentIndex]?.id] ? HeartSolidIcon : HeartIcon} 
                      className={`w-9 h-9 ${likedPosts[posts[currentIndex]?.id] ? 'text-red-500' : 'text-white'}`}
                    />
                  </view>
                  <text className={`text-white text-xs mt-0.5 ${likedPosts[posts[currentIndex]?.id] ? 'text-red-500' : ''}`}>
                    {formatNumber(likedPosts[posts[currentIndex]?.id] ? posts[currentIndex]?.likes + 1 : posts[currentIndex]?.likes)}
                  </text>
                </view>

                {/* 评论按钮 - 图标更大 */}
                <view className="flex flex-col items-center">
                  <view className="w-16 h-16 rounded-full bg-black/30 flex items-center justify-center backdrop-blur-sm cursor-pointer scale-95 transform transition-transform duration-200">
                    <image src={CommentIcon} className="w-9 h-9 text-white" />
                  </view>
                  <text className="text-white text-xs mt-0.5">
                    {formatNumber(posts[currentIndex]?.comments)}
                  </text>
                </view>

                {/* 收藏按钮 - 图标更大 */}
                <view className="flex flex-col items-center">
                  <view
                    className={`w-16 h-16 rounded-full bg-black/30 flex items-center justify-center backdrop-blur-sm cursor-pointer transform transition-all duration-200 ${
                      animatingBookmarks[posts[currentIndex]?.id] ? 'scale-125' : 'scale-95'
                    }`}
                    bindtap={() => handleBookmark(posts[currentIndex]?.id)}
                  >
                    <image 
                      src={bookmarkedPosts[posts[currentIndex]?.id] ? BookmarkSolidIcon : BookmarkIcon} 
                      className={`w-9 h-9 ${bookmarkedPosts[posts[currentIndex]?.id] ? 'text-red-500' : 'text-white'}`}
                    />
                  </view>
                  <text className={`text-white text-xs mt-0.5 ${bookmarkedPosts[posts[currentIndex]?.id] ? 'text-red-500' : ''}`}>
                    {formatNumber(bookmarkedPosts[posts[currentIndex]?.id] ? posts[currentIndex]?.bookmarks + 1 : posts[currentIndex]?.bookmarks)}
                  </text>
                
                </view>
                {/* 分享按钮 - 图标更大 */}
                <view className="flex flex-col items-center">
                  <view className="w-16 h-16 rounded-full bg-black/30 flex items-center justify-center backdrop-blur-sm cursor-pointer scale-95 transform transition-transform duration-200">
                    <image src={ShareIcon} className="w-9 h-9 text-white" />
                  </view>
                  <text className="text-white text-xs mt-0.5">
                    {formatNumber(posts[currentIndex]?.shares)}
                  </text>
                </view>
              </view>
            </view>

            {/* 滑动切换区域 - 上面1/4区域（不覆盖右侧按钮） */}
            {/* <view 
              className="absolute top-0 left-0 right-20 h-1/4 z-10"
              bindtap={() => handleSwipe("down")}
            /> */}

            {/* 滑动切换区域 - 下面1/4区域（不覆盖底部横条和右侧按钮） */}
            {/* <view 
              className="absolute bottom-24 left-0 right-20 h-1/4 z-10"
              bindtap={() => handleSwipe("up")}
            /> */}
          </view>
        )}
      </view>

      {/* 底部小横条 - 不透明的格式 */}
      <view className="w-full h-24 bg-black flex items-center justify-center">
        <view 
            className="w-fit h-24 flex items-center justify-center"
            bindtap={handleCreatePost}
        >
            <image 
                auto-size
                className="h-12 -mt-9" 
                src={AddIconOutline} 
            />
        </view>
      </view>
    </view>
  );
}