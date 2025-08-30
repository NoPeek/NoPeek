// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

# image_path=pexels-anthony-cowan-1086627-2336301
# image_path=pexels-jeff-denlea-721292-9330619
# image_path=pexels-mustapha-damilola-458083529-18561843
# image_path=anbinh-pho-t1HrGZcQybc-unsplash
image_path=max-titov-ZBZVjW4wyWk-unsplash

# detect face or plate
python detect.py -i imgs/$image_path.jpg -o jsons/face_$image_path.json -t face
python detect.py -i imgs/$image_path.jpg -o jsons/plate_$image_path.json -t plate

# given json, blur face or plate
python blur.py -i imgs/$image_path.jpg -o results/blur_face_$image_path.jpg -j jsons/face_$image_path.json -t face
# # python blur.py -i results/blur_face_$image_path.jpg -o results/blur_both_$image_path.jpg -j jsons/plate_$image_path.json -t plate
python blur.py -i imgs/$image_path.jpg -o results/blur_plate_$image_path.jpg -j jsons/plate_$image_path.json -t plate

# given json, paste sticker to the image
python sticker.py -i imgs/$image_path.jpg -o results/sticker_face_$image_path.jpg -j jsons/face_$image_path.json -t face

# given json, use Generative models to generate cartoon faces
python inpaint.py -i imgs/$image_path.jpg -o results/inpaint_face_$image_path.jpg -j jsons/face_$image_path.json -t face


image_path=agus-dietrich-eUjufrdx_bM-unsplash
# image_path=marissa-grootes-ck0i9Dnjtj0-unsplash

# detect doc for image
python blur_doc.py -i imgs/$image_path.jpg -o results/blur_doc_$image_path.jpg
