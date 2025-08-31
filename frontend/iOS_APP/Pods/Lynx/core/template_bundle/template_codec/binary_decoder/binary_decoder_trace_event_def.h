// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "core/base/lynx_trace_categories.h"

#ifndef CORE_TEMPLATE_BUNDLE_TEMPLATE_CODEC_BINARY_DECODER_BINARY_DECODER_TRACE_EVENT_DEF_H_
#define CORE_TEMPLATE_BUNDLE_TEMPLATE_CODEC_BINARY_DECODER_BINARY_DECODER_TRACE_EVENT_DEF_H_

#if ENABLE_TRACE_PERFETTO || ENABLE_TRACE_SYSTRACE

inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_SINGLE_TEMPLATE =
        "ElementBinaryReader::DecodeSingleTemplate";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_BUILTIN_ATTR_SECTION =
        "ElementBinaryReader::DecodeBuiltinAttributesSection";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_ID_SELECTOR_SECTION =
        "ElementBinaryReader::DecodeIDSelectorSection";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_INLINE_STYLES_SECTION =
        "ElementBinaryReader::DecodeInlineStylesSection";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_CLASSES_SECTION =
        "ElementBinaryReader::DecodeClassesSection";
inline constexpr const char* const ELEMENT_BINARY_READER_DECODE_EVENTS_SECTION =
    "ElementBinaryReader::DecodeEventsSection";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_PIPER_EVENTS_SECTION =
        "ElementBinaryReader::DecodePiperEventsSection";
inline constexpr const char* const ELEMENT_BINARY_READER_DECODE_ATTR_SECTION =
    "ElementBinaryReader::DecodeAttributesSection";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_DATASET_SECTION =
        "ElementBinaryReader::DecodeDatasetSection";
inline constexpr const char* const ELEMENT_BINARY_READER_CONSTRUCT_ELEMENT =
    "ElementBinaryReader::ConstructElement";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_STRING_KEY_ROUTER =
        "ElementBinaryReader::DecodeStringKeyRouter";
inline constexpr const char* const
    ELEMENT_BINARY_READER_DECODE_ORDERED_STRING_KEY_ROUTER =
        "ElementBinaryReader::DecodeOrderedStringKeyRouter";

inline constexpr const char* const BINARY_BASE_CSS_READER_DECODE_CSS_FRAGMENT =
    "DecodeCSSFragment";
inline constexpr const char* const
    BINARY_BASE_CSS_READER_DECODE_CSS_PARSE_TOKEN = "DecodeCSSParseToken";
inline constexpr const char* const
    BINARY_BASE_CSS_READER_DECODE_CSS_KEYFRAMES_TOKEN =
        "DecodeCSSKeyframesToken";
inline constexpr const char* const
    BINARY_BASE_CSS_READER_DECODE_CSS_FONT_FACE_TOKEN =
        "DecodeCSSFontFaceToken";

inline constexpr const char* const BINARY_BASE_TEMPLATE_READER_DECODE_HEADER =
    "DecodeHeader";
inline constexpr const char* const BINARY_BASE_TEMPLATE_READER_DECODE_APP_TYPE =
    "DidDecodeAppType";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_TEMPLATE_BODY = "DecodeTemplateBody";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_FLEXIBLE_TEMPLATE_BODY =
        "DecodeFlexibleTemplateBody";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_FIND_SPECIFIC_SECTION = "FindSpecificSection";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_SECTION_ROUTE = "DecodeSectionRoute";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DESERIALIZE_SECTION = "DeserializeSection";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_SPECIFIC_SECTION =
        "DecodeSpecificSection";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_PAGE_CONFIG = "DecodePageConfig";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_JS_SOURCE_SECTION =
        "DeserializeJSSourceSection";
inline constexpr const char* const
    BINARY_BASE_TEMPLATE_READER_DECODE_JS_BYTECODE_SECTION =
        "DeserializeJSBytecodeSection";

inline constexpr const char* const BINARY_READER_DID_DECODE_TEMPLATE =
    "DidDecodeTemplate";
inline constexpr const char* const BINARY_READER_DECODE_CSS_DESCRIPTOR =
    "DecodeCSSDescriptor";
inline constexpr const char* const BINARY_READER_DECODE_LEPUS_CHUNK =
    "DecodeLepusChunk";
inline constexpr const char* const BINARY_READER_DECODE_CONTEXT =
    "DecodeContext";
inline constexpr const char* const BINARY_READER_DECODE_PARSED_STYLES_SECTION =
    "DecodeParsedStylesSection";
inline constexpr const char* const
    BINARY_READER_DECODE_ELEMENT_TEMPLATE_SECTION =
        "DecodeElementTemplateSection";
inline constexpr const char* const BINARY_READER_DECODE_CUSTOM_SECTIONS =
    "DecodeCustomSections";

inline constexpr const char* const PARALLEL_READER_PARSE_ELEMENT_TEMPLATE =
    "TemplateParallelReader::ParallelParseElementTemplate";
inline constexpr const char* const PARALLEL_READER_GENERATE_PARSE_TASK =
    "TemplateParallelReader::GenerateElementTemplateParseTask";
inline constexpr const char* const PARALLEL_READER_RUN_PARSE_TASK =
    "TemplateParallelReader::RunParseElementTemplateUnitTask";
inline constexpr const char* const PARALLEL_READER_CONSTRUCT_ELEMENT =
    "TemplateParallelReader::ConstructElement";
inline constexpr const char* const PARALLEL_READER_RUN_CONSTRUCT_ELEMENT_TASK =
    "TemplateParallelReader::RunConstructElementTask";
inline constexpr const char* const PARALLEL_READER_TRY_GET_ELEMENTS =
    "TemplateParallelReader::TryGetElements";
inline constexpr const char* const PARALLEL_READER_TRY_GET_ELEMENTS_GET_LOCK =
    "TemplateParallelReader::TryGetElementsGetLock";
inline constexpr const char* const PARALLEL_READER_TRY_GET_ELEMENTS_SUCCESS =
    "TemplateParallelReader::TryGetElementsSuccess";

inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_CSS_DESCRIPTOR = "DecodeCSSDescriptor";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_CSS_DESCRIPTOR_WITH_THREAD =
        "DecodeCSSDescriptorWithThread";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_CSS_FRAGMENT_ASYNC = "DecodeCSSFragmentAsync";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_LAZY_DECODE_CSS_FRAGMENT = "LazyDecodeCSSFragment";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_LAZY_DECODE_LEPUS_CHUNK = "LazyDecodeLepusChunk";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_PARSED_STYLES_SECTION =
        "DecodeParsedStylesSection";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_ELEMENT_TEMPLATE_SECTION =
        "DecodeElementTemplateSection";
inline constexpr const char* const TEMPLATE_BINARY_READER_DECODE_LEPUS_CHUNK =
    "DecodeLepusChunk";
inline constexpr const char* const
    TEMPLATE_BINARY_READER_DECODE_LEPUS_CHUNK_ASYNC = "DecodeLepusChunkAsync";
inline constexpr const char* const TEMPLATE_BINARY_READER_COMPLETE_DECODE =
    "CompleteDecode";
inline constexpr const char* const TEMPLATE_BINARY_READER_CREATE_RECYCLER =
    "CreateTemplateBundleRecycler";
#endif  // #if ENABLE_TRACE_PERFETTO || ENABLE_TRACE_SYSTRACE

#endif  // CORE_TEMPLATE_BUNDLE_TEMPLATE_CODEC_BINARY_DECODER_BINARY_DECODER_TRACE_EVENT_DEF_H_
