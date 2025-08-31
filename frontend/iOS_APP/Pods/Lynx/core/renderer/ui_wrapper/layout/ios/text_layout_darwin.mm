// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "core/renderer/ui_wrapper/layout/ios/text_layout_darwin.h"
#include "core/renderer/dom/fiber/fiber_element.h"
#include "core/renderer/dom/fiber/image_element.h"
#include "core/renderer/dom/fiber/raw_text_element.h"
#include "core/renderer/dom/fiber/text_element.h"

#import <Lynx/LynxBaseTextShadowNode.h>
#import <Lynx/LynxConverter+UI.h>
#import <Lynx/LynxTextUtils.h>

namespace lynx {
namespace tasm {

LayoutResult TextLayoutDarwin::Measure(Element* element, float width, int width_mode, float height,
                                       int height_mode) {
  TextElement* text_element = static_cast<TextElement*>(element);
  NSMutableDictionary* childrenLayoutResultDic;
  if (text_element->need_layout_children()) {
    starlight::Constraints constraints;
    constraints[starlight::kHorizontal] =
        starlight::OneSideConstraint(width, static_cast<SLMeasureMode>(width_mode));
    constraints[starlight::kVertical] =
        starlight::OneSideConstraint(height, static_cast<SLMeasureMode>(height_mode));
    childrenLayoutResultDic = [[NSMutableDictionary alloc] init];
    MeasureChildrenRecursively(element, constraints, true, childrenLayoutResultDic);
  }

  LynxMeasureMode widthMode = (LynxMeasureMode)width_mode;
  LynxMeasureMode heightMode = (LynxMeasureMode)height_mode;
  MeasureResult result = [uiOwner_.textRenderManager measureTextWithSign:element->impl_id()
                                                                   width:width
                                                               widthMode:widthMode
                                                                  height:height
                                                              heightMode:heightMode
                                                         childrenSizeDic:childrenLayoutResultDic];
  return LayoutResult{(float)result.size.width, (float)result.size.height, (float)result.baseline};
}

void TextLayoutDarwin::MeasureChildrenRecursively(Element* element,
                                                  const starlight::Constraints& constraints,
                                                  bool final_measure,
                                                  NSMutableDictionary* childrenLayoutResultDic) {
  for (auto* child = element->first_render_child(); child; child = child->next_render_sibling()) {
    if (child->is_text()) {
      MeasureChildrenRecursively(child, constraints, final_measure, childrenLayoutResultDic);
    } else if (child->is_view() || child->is_image()) {
      FiberElement* fiber_element = static_cast<FiberElement*>(child);
      FloatSize size = fiber_element->slnode()->UpdateMeasureByPlatform(constraints, final_measure);
      [childrenLayoutResultDic setObject:@[ @(size.width_), @(size.height_), @(size.baseline_) ]
                                  forKey:@(fiber_element->impl_id())];
    }
  }
}

void TextLayoutDarwin::Align(Element* element) {
  NSDictionary* offsetDic =
      [uiOwner_.textRenderManager getInlineElementOffsetDic:element->impl_id()];
  AlignChildrenRecursively(element, offsetDic);
}

void TextLayoutDarwin::AlignChildrenRecursively(Element* element, NSDictionary* offsetDic) {
  for (auto* child = element->first_render_child(); child; child = child->next_render_sibling()) {
    if (child->is_text()) {
      AlignChildrenRecursively(child, offsetDic);
    } else if (child->is_view() || child->is_image()) {
      FiberElement* fiber_element = static_cast<FiberElement*>(child);
      id value = [offsetDic objectForKey:@(fiber_element->impl_id())];
      if (value) {
        CGPoint offset = [value CGPointValue];
        fiber_element->slnode()->AlignmentByPlatform(offset.y, offset.x);
      }
    }
  }
}

void TextLayoutDarwin::DispatchLayoutBefore(Element* element) {
  TextElement* text_element = static_cast<TextElement*>(element);
  LynxTextStyle* textStyle = [[LynxTextStyle alloc] init];
  ApplyTextStyle(text_element, textStyle);

  LynxAttributedTextBundle* textBundle = [[LynxAttributedTextBundle alloc] init];
  HandleParagraphStyle(text_element, textStyle, textBundle);

  NSDictionary<NSAttributedStringKey, id>* baseAttributes =
      [textStyle toAttributesWithFontFaceContext:uiOwner_.fontFaceContext withFontFaceObserver:nil];

  NSMutableAttributedString* attributedString = [[NSMutableAttributedString alloc] init];
  NSMutableSet* inlineElementSigns = [[NSMutableSet alloc] init];
  Boolean hasViewOrImage = NO;
  [attributedString beginEditing];
  GenerateAttributedString(attributedString, element, baseAttributes, inlineElementSigns,
                           &hasViewOrImage);
  [attributedString endEditing];
  text_element->set_need_layout_children(hasViewOrImage);

  textBundle.attributedString = attributedString;
  textBundle.inlineElementSigns = inlineElementSigns;
  textBundle.textStyle = textStyle;
  [uiOwner_.textRenderManager putAttributedTextBundle:element->impl_id() textBundle:textBundle];
}

void TextLayoutDarwin::HandleParagraphStyle(TextElement* text_element, LynxTextStyle* textStyle,
                                            LynxAttributedTextBundle* textBundle) {
  const CSSIDBitset& props_set = text_element->property_bits();
  TextProps* text_props = text_element->text_props();
  auto computed_css_style = text_element->computed_css_style();
  if (text_props) {
    if (text_props->text_max_line) {
      textBundle.maxLineNum = *text_props->text_max_line;
    }
  }
  if (props_set.Has(kPropertyIDTextOverflow)) {
    textBundle.textOverflow =
        static_cast<LynxTextOverflowType>(computed_css_style->GetTextAttributes()->text_overflow);
  }
  if (props_set.Has(kPropertyIDLineHeight)) {
    textStyle.lineHeight = computed_css_style->GetTextAttributes()->computed_line_height;
  }
  if (props_set.Has(kPropertyIDWhiteSpace)) {
    textBundle.whiteSpace =
        static_cast<LynxWhiteSpaceType>(computed_css_style->GetTextAttributes()->white_space);
  }
  if (props_set.Has(kPropertyIDTextAlign)) {
    auto value =
        static_cast<LynxTextAlignType>(computed_css_style->GetTextAttributes()->text_align);
    switch (value) {
      case LynxTextAlignLeft:
        textStyle.textAlignment = NSTextAlignmentLeft;
        break;
      case LynxTextAlignRight:
        textStyle.textAlignment = NSTextAlignmentRight;
        break;
      case LynxTextAlignCenter:
        textStyle.textAlignment = NSTextAlignmentCenter;
        break;
      case LynxTextAlignJustify:
        textStyle.textAlignment = NSTextAlignmentJustified;
        break;
      case LynxTextAlignStart:
      default:
        textStyle.textAlignment = NSTextAlignmentNatural;
        break;
    }
  }
}

void TextLayoutDarwin::GenerateAttributedString(
    NSMutableAttributedString* attributedString, Element* element,
    NSDictionary<NSAttributedStringKey, id>* baseAttributes, NSMutableSet* inlineElementSigns,
    Boolean* hasViewOrImage) {
  // handle no raw-text
  if (element->is_text()) {
    TextElement* text_element = static_cast<TextElement*>(element);
    auto element_content = text_element->content();
    if (!element_content.empty()) {
      NSString* content = [NSString stringWithUTF8String:element_content.c_str()];
      NSAttributedString* str = [[NSAttributedString alloc] initWithString:content
                                                                attributes:baseAttributes];
      [attributedString appendAttributedString:str];
    }
  }
  for (auto* child = element->first_render_child(); child; child = child->next_render_sibling()) {
    if (static_cast<FiberElement*>(child)->is_raw_text()) {
      RawTextElement* rawText = static_cast<RawTextElement*>(child);
      NSString* content = [NSString stringWithUTF8String:rawText->content().c_str()];
      NSAttributedString* str = [[NSAttributedString alloc] initWithString:content
                                                                attributes:baseAttributes];
      [attributedString appendAttributedString:str];
    } else if (child->is_text()) {
      // inline text
      TextElement* textElement = static_cast<TextElement*>(child);
      LynxTextStyle* inlineTextStyle = [[LynxTextStyle alloc] init];
      ApplyTextStyle(textElement, inlineTextStyle);

      NSMutableDictionary<NSAttributedStringKey, id>* textAttributes =
          [NSMutableDictionary dictionaryWithDictionary:baseAttributes];
      NSDictionary<NSAttributedStringKey, id>* attributes =
          [inlineTextStyle toAttributesWithFontFaceContext:uiOwner_.fontFaceContext
                                      withFontFaceObserver:nil];
      [textAttributes addEntriesFromDictionary:attributes];

      GenerateAttributedString(attributedString, textElement, textAttributes, inlineElementSigns,
                               hasViewOrImage);
    } else if (child->is_image() || child->is_view()) {
      *hasViewOrImage = YES;
      ImageElement* image_element = static_cast<ImageElement*>(child);
      LynxTextAttachment* textAttachment = [[LynxTextAttachment alloc] init];
      textAttachment.sign = image_element->impl_id();
      [inlineElementSigns addObject:@(textAttachment.sign)];
      NSMutableAttributedString* inlineElementAttributedString =
          [[NSMutableAttributedString alloc] init];
      [inlineElementAttributedString
          appendAttributedString:[NSAttributedString
                                     attributedStringWithAttachment:textAttachment]];
      [inlineElementAttributedString
          addAttributes:baseAttributes
                  range:NSMakeRange(0, inlineElementAttributedString.length)];

      [attributedString appendAttributedString:inlineElementAttributedString];
    }
  }
}

void TextLayoutDarwin::ApplyTextStyle(TextElement* text_element, LynxTextStyle* textStyle) {
  const CSSIDBitset& property_bits = text_element->property_bits();

  const auto& text_attributes = text_element->computed_css_style()->GetTextAttributes();
  if (text_attributes.has_value()) {
    for (CSSPropertyID id : property_bits) {
      switch (id) {
        case kPropertyIDFontSize:
          textStyle.fontSize =
              static_cast<float>(text_element->computed_css_style()->GetFontSize());
          break;
        case kPropertyIDColor:
          textStyle.foregroundColor =
              [LynxConverter toUIColor:@(static_cast<int>(text_attributes->color))];
          break;
        case kPropertyIDFontWeight:
          textStyle.fontWeight =
              [LynxTextUtils convertLynxFontWeight:static_cast<int>(text_attributes->font_weight)];
          break;
        case kPropertyIDFontStyle:
          textStyle.fontStyle = static_cast<LynxFontStyleType>(text_attributes->font_style);
          break;
        case kPropertyIDFontFamily:
          textStyle.fontFamilyName =
              [NSString stringWithUTF8String:(text_attributes->font_family.c_str())];
          break;
        case kPropertyIDLetterSpacing:
          textStyle.letterSpacing = text_attributes->letter_spacing;
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace tasm
}  // namespace lynx
