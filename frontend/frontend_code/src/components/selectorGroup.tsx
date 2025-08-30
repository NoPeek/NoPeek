// Copyright 2025 The NoPeek Authors. All rights reserved.
// Licensed under the Apache License, Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.


import { useState } from "@lynx-js/react";
import clsx from "clsx";
import type { CSSProperties } from "@lynx-js/types"
import CheckIcon from "../assets/icons/icon_check_outline.png";
import ShieldIcon from "../assets/icons/icon_shield_solid_white.png";

const Selector = <T extends string>(props: {
    option: { label: string, value: T, disabled?: boolean },
    isSelected: boolean,
    isStart?: boolean,
    isEnd?: boolean,
    isSingle?: boolean,
    onChange: (value: T) => void
}) => {
    const handleSelect = () => {
        if (props.option.disabled) return;
        props.onChange(props.option.value);
    };

    return (
        <view className={clsx(
            "relative flex items-center gap-3 w-full px-3 py-2",
            props.option.disabled && "opacity-35",
            props.isSelected && "bg-primary",
            props.isStart && "rounded-t-lg",
            props.isEnd && "rounded-b-lg",
            props.isSingle && "rounded-lg",
        )} bindtap={handleSelect}>
            { props.isSelected && <view className="absolute inset-0 bg-isometric" style={{ opacity: 0.1 }} /> }
            <view className="relative size-7">
                <image className={clsx(
                    "size-full",
                    props.isSelected ? "opacity-0" : "opacity-100"
                )} src={CheckIcon} />
                <image className={clsx(
                    "absolute size-full",
                    props.isSelected ? "opacity-100" : "opacity-0"
                )} src={ShieldIcon} />
            </view>
            <text className={clsx(
                "font-bold text-lg",
                props.isSelected ? "text-white" : "text-gray-800"
            )}>
                {props.option.label}&nbsp;
                { props.option.disabled && <text className="text-sm">(Coming Soon)</text> }
            </text>
        </view>
    )
};

export default function SelectorGroup<T extends string>(props: {
    className?: string,
    style?: CSSProperties,
    caption?: string,
    defaultValues?: T[],
    options: Array<{ label: string, value: T, disabled?: boolean }>,
    values: T[],
    onChange: (values: T[]) => void,
}) {
    const [selectedValues, setSelectedValues] = useState<T[]>(props.defaultValues || []);

    const handleChange = (value: T) => {
        if (selectedValues.includes(value)) {
            setSelectedValues(selectedValues.filter(v => v !== value));
        } else {
            setSelectedValues([...selectedValues, value]);
        }
        props.onChange(selectedValues);
    };

    const isSelectedArray = props.options.map(opt => selectedValues.includes(opt.value));

    return (
        <view className={clsx("flex flex-col", props.className)} style={props.style}>
            {props.options.map((option, index) => {
                const isSelected = isSelectedArray[index];
                const isPrevSelected = isSelectedArray[index - 1] ?? false;
                const isNextSelected = isSelectedArray[index + 1] ?? false;

                let isStart = false;
                let isEnd = false;
                let isSingle = false;

                if (isSelected) {
                    const isStartOfGroup = !isPrevSelected && isNextSelected;
                    const isEndOfGroup = isPrevSelected && !isNextSelected;
                    const isIsolated = !isPrevSelected && !isNextSelected;

                    isStart = isStartOfGroup;
                    isEnd = isEndOfGroup;
                    isSingle = isIsolated;
                }
                return <view key={index} className="flex items-center">
                    <Selector
                        option={option}
                        isSelected={selectedValues.includes(option.value)}
                        isStart={isStart}
                        isEnd={isEnd}
                        isSingle={isSingle}
                        onChange={handleChange}
                    />
                </view>;
            })}
            <view className="mt-2 pl-3">
                <text className="text-sm">{props.caption}</text>
            </view>
        </view>
    )
}