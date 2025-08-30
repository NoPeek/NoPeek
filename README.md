# Content

- [About NoPeek](#about-nopeek)
- [Powered by](#powered-by)
- [How to Use NoPeek](#how-to-use-nopeek)
- [Demo Video & Documentation](#demo-video--documentation)

# About NoPeek

NoPeek is an iOS application built with the Lynx frontend framework, designed to protect users’ privacy when sharing photos on social media.  
The app leverages AI-powered detection and obfuscation to automatically identify sensitive information—such as faces, license plates, ID cards, and location clues—and applies customizable protection (blur, pixelation, emoji replacement).

- 🔍 **AI Detection**: Automatically detects faces, license plates, and documents objects on photos.
- 🛡 **Multiple Privacy Protection Modes**: Provides blur, emoji substitution, and generative AI-based transformations.
- 📱 **Native iOS Experience**: Built with the Lynx framework for smooth and responsive performance.
- ☁️ **On-device Processing**: Local real-time detection and protection to safeguard user privacy.
- 📤 **Easy Social Media Integration**: Seamlessly connects with social media apps, allowing one-tap posting of privacy-protected photos directly to TikTok.


## Powered by

We carefully designed and selected tools that balance robust AI capabilities with secure infrastructure:

- **Frontend Development – Lynx**: Used to rapidly prototype and integrate AI-powered privacy features into a smooth, user-friendly interface.
- **Metadata Stripping – piexifjs**: Removes EXIF metadata (location, device, timestamps) before upload.
- **Object Detection – YOLO + InsightFace + Custom CV Pipelines**
  - [YOLOv8 License Plate Recognition]('https://github.com/Muhammad-Zeerak-Khan/Automatic-License-Plate-Recognition-using-YOLOv8')
  - [InsightFace]('https://github.com/deepinsight/insightface') for facial recognition
  - Custom CV pipelines for detecting sensitive visuals like documents and passports
- **AI Art Transformation – FLUX Diffusion Models + LoRA**
  - [FLUX Diffusion Model]('https://huggingface.co/black-forest-labs/FLUX.1-Depth-dev') for stylized avatars
  - [LoRA Collection]('https://huggingface.co/XLabs-AI/flux-lora-collection?utm_source=chatgpt.com') for personalization
- **Backend Infrastructure – FastAPI + MySQL**: Fast, lightweight APIs with secure, scalable database management.

# How to Use NoPeek

## Requirements

NoPeek is developed for iOS devices and built on macOS environments. Below are the key requirements:

- Target Platform: iOS 18.6+
- Development OS: macOS (Frontend) and Ubuntu 22.04 (Backend)
- Build Tool: Xcode 14+
- Frontend: Node.js and pnpm
- Backend: Python 3.12+ with FastAPI
- Database: MySQL

## Getting Started

Follow these steps to set up and run NoPeek locally:

1. Clone the Repository

    ```bash
    git clone https://github.com/your-team/PhotoSafer.git
    cd nopeek
    ```

2. Install Dependencies

- Backend (FastAPI + Python)

    ```bash
    cd backend
    pip install -r requirements.txt
    ```

- Frontend (Lynx + Node.js)

    ```bash
    cd frontend
    pnpm install
    ```

3. Run the Project

- Backend

    ```bash
    cd backend
    python -m uvicorn test:app --reload --host 0.0.0.0 --port 8000
    ```

- Frontend

    ```bash
    cd frontend
    pnpm run dev
    ```

4. Open the App
- Access backend APIs at: http://127.0.0.1:8000/docs
- Access the iOS app in the simulator via Xcode.

# Demo Video & Documentation

Our demo video is available on YouTube 👉 [Watch Here]('https://youtube.com').

For a deeper dive into NoPeek, check out the full 📄 [Project Documentation](Your-Docs-Link).