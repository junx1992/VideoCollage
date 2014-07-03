#ifndef __XTL_HELPER_HPP__
#define __XTL_HELPER_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:

Abstract:
    General helper functions and classes
Notes:

Usage:
    1. IProgress interface

History:
    Created  on 2004 June 6 by oliver_liyin
    2004 Jul 19. by liyin, Merged with xtl_traits for better organization

\*************************************************************************/

#include <vector>
#include <tchar.h>

namespace xtl
{


    class IProgress
    {
    public:
        /// dtor is virtual to be inherited
        virtual ~IProgress() {}

        /// Set the value of current progress between min and max value
        virtual void SetValue(int value, int minValue, int maxValue)
        {
            for(size_t k = 0; k < m_rangeMapping.size(); k ++)
            {
                MapRange(value, minValue, maxValue, m_rangeMapping[k]);
            }

            RealizeValue(value, minValue, maxValue);
        }

        /// Push a range mapping in stack.
        /// The value and range in function SetValue function will be mapped to
        ///      the subRange, and then mapped to entireRange.
        /// And the entireRange will be mapped to next subRange in entireRange, and so on.
        virtual void PushRangeMapping(int startValue, int endValue, int minValue, int maxValue)
        {
            RangeMapping mapping = {startValue, endValue, minValue, maxValue};
            m_rangeMapping.push_back(mapping);
        }

        /// Pop the last range mapping
        virtual void PopRangeMapping()
        {
            m_rangeMapping.pop_back();
        }

        /// Set the title of the progress
        virtual void SetMessage(const TCHAR* /*message*/)
        {
            /// default ignores the message
        }

    protected:
        /// Override this function to realize the value and range on UI, or in console
        virtual void RealizeValue(int value, int minValue, int maxValue) = 0;

    private:
        struct RangeMapping
        {
            int startValue, endValue, minValue, maxValue;
        };

        void MapRange(int& value, int& minValue, int& maxValue, const RangeMapping& mapping)
        {
            const int oldExpand = maxValue - minValue;
            const int newExpand = mapping.endValue - mapping.startValue;
            value = mapping.startValue + (value - minValue) * newExpand / oldExpand;
            minValue = mapping.minValue;
            maxValue = mapping.maxValue;
        }

        std::vector<RangeMapping> m_rangeMapping;
    };

}   // namespace xtl


#endif//__XTL_HELPER_HPP__
