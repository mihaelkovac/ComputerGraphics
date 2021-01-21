#ifndef MVK_QUEUE_H
#define MVK_QUEUE_H


#include <vulkan/vulkan.h>
#include <array>
#include <vector>

#include "utils.h"

namespace mvk
{

    enum class FamilyType
    {
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT,
        SparseBinding = VK_QUEUE_SPARSE_BINDING_BIT,
        Protected = VK_QUEUE_PROTECTED_BIT,
        Presentation = VK_QUEUE_PROTECTED_BIT << 1,
        Undefined = VK_QUEUE_FLAG_BITS_MAX_ENUM
    };


    namespace _detail
    {

        union QueueIndex
        {
            uint32_t _full_val = 0;
            struct
            {
                uint16_t index;
                uint8_t  _padd;
                uint8_t  found;
            };

        };

        template<FamilyType>
        struct QueueFamilyIndex
        {
            // static_assert(false, "Cannot use undefined as index of queue family");
        };

#define _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(_type, _member) \
            template<>                                                \
            struct QueueFamilyIndex<FamilyType::_type>                \
            {                                                         \
            protected:                                                \
                [[nodiscard]] inline uint32_t Get() const noexcept    \
                {                                                     \
                    return static_cast<uint32_t>(_member.index);      \
                }                                                     \
                                                                      \
                [[nodiscard]] inline bool Found() const noexcept      \
                {                                                     \
                    return static_cast<bool>(_member.found);          \
                }                                                     \
                                                                      \
                inline void Set(uint32_t index) noexcept              \
                {                                                     \
                    _member.index = index;                            \
					_member.found = index != UINT32_MAX;              \
				}                                                     \
                QueueIndex _member;                                   \
            }                                                         \

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Compute, compute_family);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Graphics, graphics_family);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Transfer, transfer_family);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(SparseBinding, sparse_binding_family);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Protected, protected_family);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Presentation, presentation_family);

    } // namespace _detail

    template<FamilyType ... Families>
    struct QueueFamilyIndices : _detail::QueueFamilyIndex<Families>...
    {
        static constexpr size_t FamilyCount = sizeof...(Families);

        template<FamilyType Type>
        [[nodiscard]] constexpr inline bool Found() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot inspect Found value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::Found();
        }

        template<FamilyType Type>
        [[nodiscard]] constexpr inline uint32_t Get() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot Get index value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::Get();
        }

        template<FamilyType Type>
        [[nodiscard]] constexpr inline bool Unique() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot test uniqueness of family index which was not picked");
            return ((_detail::QueueFamilyIndex<Type>::Get() == _detail::QueueFamilyIndex<Families>::Get()) + ...) == 1;
        }


        [[nodiscard]] constexpr inline bool AllFamiliesFound() const noexcept
        {
            return (_detail::QueueFamilyIndex<Families>::Found() && ...);
        }

        template<FamilyType Type>
        constexpr inline void Set(uint32_t index) noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot Set index value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::Set(index);
        }
    };


    namespace _detail
    {
        template<FamilyType>
        struct SingleQueueFamilyPolicy
        {
            // static_assert(false, "Cannot use undefined as index of queue family");
        };

#define _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(_type, _member)                                       \
            template<>                                                                                                     \
            struct SingleQueueFamilyPolicy<FamilyType::_type>                                                              \
            {                                                                                                              \
            public:                                                                                                        \
                template<FamilyType ... Families>                                                                          \
                void Init(VkDevice device, const QueueFamilyIndices<Families...>& indices) noexcept                        \
                {                                                                                                          \
                    _member ## _index = indices.template Get<FamilyType::_type>();                                         \
                    vkGetDeviceQueue(device, _member ## _index, 0, &_member ## _queue);                                    \
                }                                                                                                          \
                [[nodiscard]] inline constexpr uint32_t GetIndex() const noexcept                                          \
                {                                                                                                          \
                    return _member ## _index;                                                                              \
                }                                                                                                          \
                [[nodiscard]] inline constexpr VkQueue GetVkQueue() const noexcept                                         \
                {                                                                                                          \
                    return _member ## _queue;                                                                              \
                }                                                                                                          \
                VkQueue  _member ## _queue = VK_NULL_HANDLE;                                                               \
                uint32_t _member ## _index = UINT32_MAX;                                                                   \
            }                                                                                                              \

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Compute, compute_family);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Graphics, graphics_family);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Transfer, transfer_family);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(SparseBinding, sparse_binding_family);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Protected, protected_family);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Presentation, presentation_family);


        template<FamilyType Family, size_t QueueCount>
        struct MultiQueueFamilyPolicy
        {

        };


#define _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(_type, _member)                   \
		template<size_t QueueCount>                                                               \
    	struct MultiQueueFamilyPolicy<FamilyType::_type, QueueCount>                              \
    	{                                                                                         \
    	public:                                                                                   \
    		template<FamilyType ... Families>                                                     \
            void Init(VkDevice device, const QueueFamilyIndices<Families...>& indices) noexcept   \
            {                                                                                     \
				_member ## _index = indices.template Get<FamilyType::_type>();                    \
                for (size_t i = 0; i < QueueCount; ++i)                                           \
                {                                                                                 \
                    vkGetDeviceQueue(device, _member ## _index, i, &_member ## _queues[i]);       \
                }                                                                                 \
            }                                                                                     \
            [[nodiscard]] inline constexpr uint32_t GetIndex() const noexcept                     \
            {                                                                                     \
                return _member ## _index;                                                         \
            }                                                                                     \
            [[nodiscard]] inline constexpr VkQueue GetVkQueue(const size_t index) const noexcept  \
            {                                                                                     \
                return _member ## _queues[index];                                                 \
            }                                                                                     \
            std::array<VkQueue, QueueCount> _member ## _queues{};                                 \
            uint32_t _member ## _index = UINT32_MAX;                                              \
    	}                                                                                         \

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(Compute, compute_family);

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(Graphics, graphics_family);

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(Transfer, transfer_family);

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(SparseBinding, sparse_binding_family);

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(Protected, protected_family);

        _MVK_DETAIL_DEFINE_MULTI_QUEUE_FAMILY_POLICY_STRUCT(Presentation, presentation_family);
    } // namespace _detail


    template<FamilyType ... Families>
    struct DefaultQueuePolicy : public _detail::SingleQueueFamilyPolicy<Families>...
    {

        template<typename FamilyIndices>
        void Init(VkDevice device, const FamilyIndices& indices) noexcept
        {
            (_detail::SingleQueueFamilyPolicy<Families>::Init(device, indices), ...);
        }

        template<FamilyType Type>
        VkResult SubmitToQueue(VkSubmitInfo* submit_infos, uint32_t submit_count, VkFence fence = VK_NULL_HANDLE) noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return vkQueueSubmit(_detail::SingleQueueFamilyPolicy<Type>::GetVkQueue(), submit_count, submit_infos, fence);
        }

        template<FamilyType Type>
        VkQueue GetVkQueue() noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return _detail::SingleQueueFamilyPolicy<Type>::GetVkQueue();
        }

        template<FamilyType Type>
        uint32_t GetQueueFamilyIndex() noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return _detail::SingleQueueFamilyPolicy<Type>::GetIndex();
        }

    };


    struct QueueFamilyPolicyBase
    {
        static inline bool CanPresent(VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t family_index) noexcept
        {
            VkBool32 result = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, family_index, surface, &result);

            return result;
        }

        template<FamilyType ... Families>
        static inline uint32_t GetUniqueIndices(const QueueFamilyIndices<Families...>& indices, ::std::array<uint32_t, sizeof...(Families)>& unique_indices)
        {
            uint64_t taken_indices_flags = 0;
            auto unique_ind_it = unique_indices.begin();

            return (GetUniqueIndex<Families>(indices, taken_indices_flags, unique_ind_it) + ...);
        }


    protected:

        template<FamilyType Family, typename QIndices, typename It>
        static uint32_t GetUniqueIndex(const QIndices& indices, uint64_t& taken_indices, It& unique_indices_it)
        {
            uint32_t index = indices.template Get<Family>();
            if (taken_indices & (1 << index))
            {
                return 0;
            }

            taken_indices |= 1 << index;
            *unique_indices_it = index;
            ++unique_indices_it;
            return 1;
        }
    };



    template<bool RequireCompute>
    struct DefaultQueueFamilyPolicy : public QueueFamilyPolicyBase
    {
        using QueueFamilyIndicesT = typename util::TernaryTypeOp<
            RequireCompute,
            QueueFamilyIndices<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>, // :
            QueueFamilyIndices<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>
        >::Type;

        using QueuePolicy = typename util::TernaryTypeOp<RequireCompute,
            DefaultQueuePolicy<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>,
            DefaultQueuePolicy<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>
        >::Type;

        template<typename GPUInfo, typename SurfaceInfo>
        [[nodiscard]] QueueFamilyIndicesT FindQueueFamilies(const GPUInfo& gpu, const SurfaceInfo& surface) const noexcept
        {
            QueueFamilyIndicesT indices{};

            for (uint32_t index = 0; index < gpu.supported_queue_families.size(); ++index)
            {
                if (gpu.isFamilyOfType(index, FamilyType::Graphics) && !indices.template Found<FamilyType::Graphics>())
                {
                    indices.template Set<FamilyType::Graphics>(index);
                }
                else if (gpu.isFamilyOfType(index, FamilyType::Transfer) && !indices.template Found<FamilyType::Transfer>())
                {
                    indices.template Set<FamilyType::Transfer>(index);
                }

                if (CanPresent(gpu.vk_gpu, surface.vk_surface, index) && !indices.template Found<FamilyType::Presentation>())
                {
                    indices.template Set<FamilyType::Presentation>(index);
                }

                if constexpr (RequireCompute)
                {
                    //if(index != indices.Get<FamilyType::Graphics>() && gpu.isFamilyOfType(index, FamilyType::Compute))
                    //{
                    //    indices.Set<FamilyType::Compute>(index);
                    //}

                    if (gpu.isFamilyOfType(index, FamilyType::Compute))
                    {
                        indices.template Set<FamilyType::Compute>(index);
                    }
                }

                if (indices.AllFamiliesFound())
                {
                    break;
                }

            }

            if (!indices.template Found<FamilyType::Transfer>() && indices.template Found<FamilyType::Graphics>())
            {
                indices.template Set<FamilyType::Transfer>(indices.template Get<FamilyType::Graphics>());
            }

            return indices;
        }

        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> GetQueueInfos(const QueueFamilyIndicesT& indices) const noexcept
        {
            std::vector<VkDeviceQueueCreateInfo> infos{};

            std::array<uint32_t, QueueFamilyIndicesT::FamilyCount> unique_indices{};
            size_t unique_index_count = GetUniqueIndices(indices, unique_indices);

            for (size_t i = 0; i < unique_index_count; ++i)
            {
                VkDeviceQueueCreateInfo& info = infos.emplace_back();
                info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                info.queueCount = 1;
                info.queueFamilyIndex = unique_indices[i];
                info.pQueuePriorities = &priority;
            }


            return infos;

        }

    private:
        float priority = 1.0f;
    };



} // namespace mvk

#endif