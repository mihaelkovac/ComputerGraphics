#ifndef MVK_QUEUE_H
#define MVK_QUEUE_H


#include <vulkan/vulkan.h>
#include <array>

namespace mvk
{

    enum class FamilyType
    {
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute  = VK_QUEUE_COMPUTE_BIT,
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
            uint32_t _fullVal = 0;
            struct
            {
                uint16_t index;
                uint8_t  _padd;
                uint8_t  found;
            };
            
        };

        #define _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(_type, _member) \
            template<>                                                \
            struct QueueFamilyIndex<FamilyType::_type>                \
            {                                                         \
            protected:                                                \
                inline uint32_t get() const noexcept                  \
                {                                                     \
                    return static_cast<uint32_t>(_member.index);      \
                }                                                     \
                                                                      \
                inline bool found() const noexcept                    \
                {                                                     \
                    return static_cast<bool>(_member.found);          \
                }                                                     \
                                                                      \
                inline void set(uint32_t index) noexcept              \
                {                                                     \
                    _member.index = index;                            \
					_member.found = index != UINT32_MAX;              \
				}                                                     \
                QueueIndex _member;                                   \
            }                                                         \

        template<FamilyType>
        struct QueueFamilyIndex
        {
            // static_assert(false, "Cannot use undefined as index of queue family");
        };
        
        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Compute, computeFamily);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Graphics, graphicsFamily);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Transfer, transferFamily);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(SparseBinding, sparseBindingFamily);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Protected, protectedFamily);

        _MVK_DETAIL_DEFINE_QUEUE_INDEX_STRUCT(Presentation, presentationFamily);

    } // namespace _detail
    
    template<FamilyType ... Families>
    struct QueueFamilyIndices : _detail::QueueFamilyIndex<Families>...
    {
        static constexpr size_t FamilyCount = sizeof...(Families);

        template<FamilyType Type>
        constexpr inline bool found() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot inspect found value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::found();   
        }

        template<FamilyType Type>
        constexpr inline uint32_t get() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot get index value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::get();
        }

        template<FamilyType Type>
        constexpr inline bool unique() const noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot test uniqueness of family index which was not picked");
            return ((_detail::QueueFamilyIndex<Type>::get() == _detail::QueueFamilyIndex<Families>::get()) + ...) == 1;
        }


        constexpr inline bool allFamiliesFound() const noexcept
        {
            return (_detail::QueueFamilyIndex<Families>::found() && ...);
        }

        template<FamilyType Type>
        constexpr inline void set(uint32_t index) noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot set index value for queue family which was not specified for this template specialization");
            return _detail::QueueFamilyIndex<Type>::set(index);
        }
    };


    namespace _detail
    {
        #define _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(_type, _member)                                       \
            template<>                                                                                                     \
            struct SingleQueueFamilyPolicy<FamilyType::_type>                                                              \
            {                                                                                                              \
            public:                                                                                                        \
                template<FamilyType ... Families>                                                                          \
                void init(VkDevice device, const QueueFamilyIndices<Families...>& indices) noexcept                        \
                {                                                                                                          \
                    _member ## Index = indices.template get<FamilyType::_type>();                                          \
                    vkGetDeviceQueue(device, _member ## Index, 0, &_member ## Queue);                                      \
                }                                                                                                          \
                inline constexpr uint32_t getIndex() const noexcept                                                        \
                {                                                                                                          \
                    return _member ## Index;                                                                               \
                }                                                                                                          \
                inline constexpr VkQueue getVkQueue() const noexcept                                                       \
                {                                                                                                          \
                    return _member ## Queue;                                                                               \
                }                                                                                                          \
                VkQueue  _member ## Queue = VK_NULL_HANDLE;                                                                \
                uint32_t _member ## Index = UINT32_MAX;                                                                    \
            }                                                                                                              \

        template<FamilyType>
        struct SingleQueueFamilyPolicy
        {
            // static_assert(false, "Cannot use undefined as index of queue family");
        };

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Compute, computeFamily);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Graphics, graphicsFamily);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Transfer, transferFamily);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(SparseBinding, sparseBindingFamily);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Protected, protectedFamily);

        _MVK_DETAIL_DEFINE_SINGLE_QUEUE_FAMILY_POLICY_STRUCT(Presentation, presentationFamily);
        
    } // namespace _detail
    

    template<FamilyType ... Families>
    struct DefaultQueuePolicy : public _detail::SingleQueueFamilyPolicy<Families>...
    {
        
        template<typename FamilyIndices>
        void init(VkDevice device, const FamilyIndices& indices) noexcept
        {
            (_detail::SingleQueueFamilyPolicy<Families>::init(device, indices), ...);
        }

        template<FamilyType Type>
        VkResult submitToQueue(VkSubmitInfo* submitInfos, uint32_t submitCount, VkFence fence = VK_NULL_HANDLE) noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return vkQueueSubmit(_detail::SingleQueueFamilyPolicy<Type>::getVkQueue(), submitCount, submitInfos, fence);
        }

        template<FamilyType Type>
        VkQueue getVkQueue() noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return _detail::SingleQueueFamilyPolicy<Type>::getVkQueue();
        }

        template<FamilyType Type>
        uint32_t getQueueFamilyIndex() noexcept
        {
            static_assert(((Type == Families) || ...), "Cannot submit to queue which was not specified for this template specialization");
            return _detail::SingleQueueFamilyPolicy<Type>::getIndex();
        }

    };


    struct QueueFamilyPolicyBase
    {
        static inline bool canPresent(VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t familyIndex) noexcept
        {
            VkBool32 result = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familyIndex, surface, &result);

            return result;
        }

        template<FamilyType ... Families>
        static inline uint32_t getUniqueIndices(const QueueFamilyIndices<Families...>& indices, std::array<uint32_t, sizeof...(Families)>& uniqueIndices)
        {
            uint64_t takenIndicesFlags = 0;
            auto uniqueIndIt = uniqueIndices.begin();

            return (getUniqueIndex<Families>(indices, takenIndicesFlags, uniqueIndIt) + ...);
        }


    protected:

        template<FamilyType Family, typename QIndices, typename It>
        static uint32_t getUniqueIndex(const QIndices& indices, uint64_t& takenIndices, It& uniqueIndicesIt)
        {
            uint32_t index = indices.template get<Family>();
            if(takenIndices & (1 << index))
            {
                return 0;
            }

            takenIndices |= 1 << index;
            *uniqueIndicesIt = index;
            uniqueIndicesIt++;
            return 1;
        }
    };



    template<bool RequireCompute>
    struct DefaultQueueFamilyPolicy : public QueueFamilyPolicyBase
    {
        using QueueFamilyIndicesT = typename util::TernaryTypeOp<
                                             RequireCompute, // ?
                                             QueueFamilyIndices<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>, // :
                                             QueueFamilyIndices<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>
                                                       >::Type;
        
        using QueuePolicy = typename util::TernaryTypeOp<
                                                RequireCompute, 
                                                DefaultQueuePolicy<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>,
                                                DefaultQueuePolicy<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>
                                                        >;
        template<typename GPUInfo, typename SurfaceInfo>
        QueueFamilyIndicesT findQueueFamilies(const GPUInfo& gpu, const SurfaceInfo& surface) const noexcept
        {   
            QueueFamilyIndicesT indices{};

            for(uint32_t index = 0; index < gpu.supportedQueueFamilies.size(); ++index)
            {
                if(gpu.isFamilyOfType(index, FamilyType::Graphics) && !indices.found<FamilyType::Graphics>())
                {
                    indices.set<FamilyType::Graphics>(index);
                }
                else if(gpu.isFamilyOfType(index, FamilyType::Transfer) && !indices.found<FamilyType::Transfer>())
                {
                    indices.set<FamilyType::Transfer>(index);
                }
                
                if(canPresent(gpu.vkGPU, surface.vkSurface, index) && !indices.found<FamilyType::Presentation>())
                {
                    indices.set<FamilyType::Presentation>(index);
                }

                if constexpr(RequireCompute)
                {
                    //if(index != indices.get<FamilyType::Graphics>() && gpu.isFamilyOfType(index, FamilyType::Compute))
                    //{
                    //    indices.set<FamilyType::Compute>(index);
                    //}

                	if(gpu.isFamilyOfType(index, FamilyType::Compute))
                	{
                        indices.set<FamilyType::Compute>(index);
                	}
                }

                if(indices.allFamiliesFound())
                {
                    break;
                }
                
            }

            if(!indices.template found<FamilyType::Transfer>() && indices.template found<FamilyType::Graphics>())
            {
                indices.template set<FamilyType::Transfer>(indices.template get<FamilyType::Graphics>());
            }

            return indices;
        }

        std::vector<VkDeviceQueueCreateInfo> getQueueInfos(const QueueFamilyIndicesT& indices) const noexcept
        {
            std::vector<VkDeviceQueueCreateInfo> infos{};
            
            std::array<uint32_t, QueueFamilyIndicesT::FamilyCount> uniqueIndices{};
            size_t uniqueIndexCount = getUniqueIndices(indices, uniqueIndices);

            for(size_t i = 0; i < uniqueIndexCount; ++i)
            {
                VkDeviceQueueCreateInfo& info = infos.emplace_back();
                info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                info.queueCount = 1;
                info.queueFamilyIndex = uniqueIndices[i];
                info.pQueuePriorities = &priority;
            }


            return infos;
            
        }

    private:
        float priority = 1.0f;
    };


    
} // namespace mvk


#endif