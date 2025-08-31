// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef CORE_STYLE_LAYOUT_PROPERTY_H_
#define CORE_STYLE_LAYOUT_PROPERTY_H_

#include "core/renderer/css/css_property.h"

namespace lynx {
namespace tasm {

#define FOREACH_LAYOUT_PROPERTY(V)      V(AlignContent, LAYOUT_ONLY)                 \
  V(AlignItems, LAYOUT_ONLY)                 \
  V(AlignSelf, LAYOUT_ONLY)                 \
  V(AspectRatio, LAYOUT_ONLY)                 \
  V(Border, LAYOUT_WANTED)                 \
  V(BorderBottom, LAYOUT_WANTED)                 \
  V(BorderBottomWidth, LAYOUT_WANTED)                 \
  V(BorderLeft, LAYOUT_WANTED)                 \
  V(BorderLeftWidth, LAYOUT_WANTED)                 \
  V(BorderRight, LAYOUT_WANTED)                 \
  V(BorderRightWidth, LAYOUT_WANTED)                 \
  V(BorderTop, LAYOUT_WANTED)                 \
  V(BorderTopWidth, LAYOUT_WANTED)                 \
  V(BorderWidth, LAYOUT_WANTED)                 \
  V(Bottom, LAYOUT_ONLY)                 \
  V(BoxSizing, LAYOUT_ONLY)                 \
  V(ColumnGap, LAYOUT_ONLY)                 \
  V(Content, LAYOUT_ONLY)                 \
  V(Direction, LAYOUT_WANTED)                 \
  V(Display, LAYOUT_ONLY)                 \
  V(Flex, LAYOUT_ONLY)                 \
  V(FlexBasis, LAYOUT_ONLY)                 \
  V(FlexDirection, LAYOUT_ONLY)                 \
  V(FlexGrow, LAYOUT_ONLY)                 \
  V(FlexShrink, LAYOUT_ONLY)                 \
  V(FlexWrap, LAYOUT_ONLY)                 \
  V(FontFeatureSettings, LAYOUT_WANTED)                 \
  V(FontOpticalSizing, LAYOUT_WANTED)                 \
  V(FontVariationSettings, LAYOUT_WANTED)                 \
  V(Gap, LAYOUT_ONLY)                 \
  V(GridAutoColumns, LAYOUT_ONLY)                 \
  V(GridAutoFlow, LAYOUT_ONLY)                 \
  V(GridAutoRows, LAYOUT_ONLY)                 \
  V(GridColumnEnd, LAYOUT_ONLY)                 \
  V(GridColumnGap, LAYOUT_ONLY)                 \
  V(GridColumnSpan, LAYOUT_ONLY)                 \
  V(GridColumnStart, LAYOUT_ONLY)                 \
  V(GridRowEnd, LAYOUT_ONLY)                 \
  V(GridRowGap, LAYOUT_ONLY)                 \
  V(GridRowSpan, LAYOUT_ONLY)                 \
  V(GridRowStart, LAYOUT_ONLY)                 \
  V(GridTemplateColumns, LAYOUT_ONLY)                 \
  V(GridTemplateRows, LAYOUT_ONLY)                 \
  V(Height, LAYOUT_ONLY)                 \
  V(JustifyContent, LAYOUT_ONLY)                 \
  V(JustifyItems, LAYOUT_ONLY)                 \
  V(JustifySelf, LAYOUT_ONLY)                 \
  V(Left, LAYOUT_ONLY)                 \
  V(LinearCrossGravity, LAYOUT_ONLY)                 \
  V(LinearDirection, LAYOUT_ONLY)                 \
  V(LinearGravity, LAYOUT_ONLY)                 \
  V(LinearLayoutGravity, LAYOUT_ONLY)                 \
  V(LinearOrientation, LAYOUT_ONLY)                 \
  V(LinearWeight, LAYOUT_ONLY)                 \
  V(LinearWeightSum, LAYOUT_ONLY)                 \
  V(ListCrossAxisGap, LAYOUT_WANTED)                 \
  V(Margin, LAYOUT_ONLY)                 \
  V(MarginBottom, LAYOUT_ONLY)                 \
  V(MarginLeft, LAYOUT_ONLY)                 \
  V(MarginRight, LAYOUT_ONLY)                 \
  V(MarginTop, LAYOUT_ONLY)                 \
  V(MaxHeight, LAYOUT_ONLY)                 \
  V(MaxWidth, LAYOUT_ONLY)                 \
  V(MinHeight, LAYOUT_ONLY)                 \
  V(MinWidth, LAYOUT_ONLY)                 \
  V(Order, LAYOUT_ONLY)                 \
  V(Padding, LAYOUT_ONLY)                 \
  V(PaddingBottom, LAYOUT_ONLY)                 \
  V(PaddingLeft, LAYOUT_ONLY)                 \
  V(PaddingRight, LAYOUT_ONLY)                 \
  V(PaddingTop, LAYOUT_ONLY)                 \
  V(Perspective, LAYOUT_WANTED)                 \
  V(Position, LAYOUT_ONLY)                 \
  V(RelativeAlignBottom, LAYOUT_ONLY)                 \
  V(RelativeAlignLeft, LAYOUT_ONLY)                 \
  V(RelativeAlignRight, LAYOUT_ONLY)                 \
  V(RelativeAlignTop, LAYOUT_ONLY)                 \
  V(RelativeBottomOf, LAYOUT_ONLY)                 \
  V(RelativeCenter, LAYOUT_ONLY)                 \
  V(RelativeId, LAYOUT_ONLY)                 \
  V(RelativeLayoutOnce, LAYOUT_ONLY)                 \
  V(RelativeLeftOf, LAYOUT_ONLY)                 \
  V(RelativeRightOf, LAYOUT_ONLY)                 \
  V(RelativeTopOf, LAYOUT_ONLY)                 \
  V(Right, LAYOUT_ONLY)                 \
  V(RowGap, LAYOUT_ONLY)                 \
  V(Top, LAYOUT_ONLY)                 \
  V(VerticalAlign, LAYOUT_WANTED)                 \
  V(Width, LAYOUT_ONLY)                 

enum ConsumptionStatus { LAYOUT_ONLY = 0, LAYOUT_WANTED = 1, SKIP = 2 };

class LayoutProperty {
 public:
  LayoutProperty() = delete;
  ~LayoutProperty() = delete;

  static ConsumptionStatus ConsumptionTest(CSSPropertyID id);
  inline static bool IsLayoutOnly(CSSPropertyID id) {
    return ConsumptionTest(id) == LAYOUT_ONLY;
  }
  inline static bool IsLayoutWanted(CSSPropertyID id) {
    return ConsumptionTest(id) == LAYOUT_WANTED;
  }
};

}  // namespace tasm
}  // namespace lynx

#endif  // CORE_STYLE_LAYOUT_PROPERTY_H_
