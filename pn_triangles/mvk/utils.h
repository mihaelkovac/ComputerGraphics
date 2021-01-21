#ifndef MVK_UTILS_H
#define MVK_UTILS_H

#include <vulkan/vulkan.h>
#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <stdint.h>

#define MVK_MACRO_BEGIN do {
#define MVK_MACRO_END } while(0)
#define MVK_CONST_FUN __attribute__((const)) constexpr

#define MVK_VALIDATE_RESULT(_result, _error_message) assert((VK_SUCCESS == _result) && _error_message)
#define MVK_CHECK_FATAL(_cond, _msg) \
    MVK_MACRO_BEGIN                  \
    if(!_cond)                       \
    {                                \
        fprintf(stderr, _msg);       \
        abort();                     \
                                     \
    }                                \
    MVK_MACRO_END                    \

#define MVK_UNDERLYING(_val) static_cast<std::underlying_type_t<decltype(_val)>>(_val)

#define MVK_RESULT_T(_func, ...) std::invoke_result_t<_func, ##__VA_ARGS__>


namespace std
{
    template<>
    struct tuple_size<::VkExtent2D> : integral_constant<size_t, 2> {};

    template<>
    struct tuple_element<0, ::VkExtent2D>
    {
        using type = decltype(VkExtent2D::width);
    };

    template<>
    struct tuple_element<1, ::VkExtent2D>
    {
        using type = decltype(VkExtent2D::height);
    };


    template<>
    struct tuple_size<::VkExtent3D> : integral_constant<size_t, 3> {};

    template<>
    struct tuple_element<0, ::VkExtent3D>
    {
        using type = decltype(::VkExtent3D::width);
    };

    template<>
    struct tuple_element<1, ::VkExtent3D>
    {
        using type = decltype(::VkExtent3D::height);
    };

    template<>
    struct tuple_element<2, ::VkExtent3D>
    {
        using type = decltype(::VkExtent3D::depth);
    };

    
} // namespace std


template<size_t Index>
std::tuple_element_t<Index, VkExtent2D> get(const VkExtent2D& extent) noexcept
{
    if constexpr(Index == 0) return extent.width;
    if constexpr(Index == 1) return extent.height;
}


template<size_t Index>
std::tuple_element_t<Index, VkExtent3D> get(const VkExtent3D& extent) noexcept
{
    if constexpr(Index == 0) return extent.width;
    if constexpr(Index == 1) return extent.height;
    if constexpr(Index == 2) return extent.depth;
}

namespace mvk
{

    namespace util
    {

    	struct BinaryData
    	{
            uint8_t* data{ nullptr };
            size_t size{ 0 };
    	};

        BinaryData ReadFile(const char* filename)
        {
            FILE* file = nullptr;
        	
            fopen_s(&file, filename, "rb");
        	
            BinaryData data{};
            if (!file)
            {
                return data;
            }
        	
        	
            fseek(file, 0L, SEEK_END);
            data.size = ftell(file);
            fseek(file, 0L, SEEK_SET);

            data.data = new uint8_t[data.size];
            fread(data.data, 1, data.size, file);

            return data;
        }
    	
        template<typename Enum>
        constexpr inline std::underlying_type_t<Enum> Underlying(const Enum value) noexcept
        {
            return static_cast<std::underlying_type_t<Enum>>(value);
        }

        
        constexpr inline uint32_t Version(int major, int minor, int patch)
        {
            return VK_MAKE_VERSION(major, minor, patch);
        }
        
        template<bool Condition, typename First, typename Second>
        struct TernaryTypeOp;

        template<typename First, typename Second>
        struct TernaryTypeOp<true, First, Second>
        {
            using Type = First;
        };

        template<typename First, typename Second>
        struct TernaryTypeOp<false, First, Second>
        {
            using Type = Second;
        };

        struct Void{};

        template<typename Enum,
                 typename Flags = uint32_t>
        struct EnumFlags
        {
            
            constexpr EnumFlags() = default;

            constexpr EnumFlags(Enum initalVal) noexcept : flags(util::Underlying(initalVal))
            {
            }

            constexpr EnumFlags(Flags initialValue) noexcept : flags(initialValue)
            {
            }

            constexpr EnumFlags(std::initializer_list<Enum> vals) noexcept
            {
                for(Enum val : vals)
                {
                    Set(val);
                }
            }

            constexpr bool Test(Enum val) const noexcept
            {
                return flags & util::Underlying(val);
            }

            constexpr void Set(Enum val) noexcept
            {
                flags |= util::Underlying(val);
            }

            constexpr void Unset(Enum val) noexcept
            {
                flags &= ~util::Underlying(val);
            }

            constexpr bool Contains(const EnumFlags& other) const noexcept
            {
                return (flags & other.flags) == other.flags;
            }

        	constexpr void Reset() const noexcept
            {
                flags = 0;
            }

            constexpr operator Flags() const noexcept
            {
                return flags;
            }

            constexpr EnumFlags operator|(EnumFlags other) const noexcept
            {
                return EnumFlags{ other.flags | flags };
            }

        	constexpr EnumFlags& operator|=(EnumFlags other) const noexcept
            {
                flags |= other.flags;
                return *this;
            }

            constexpr EnumFlags operator|(Enum e) const noexcept
            {
                return EnumFlags{ flags | Underlying(e) };
            }

        	constexpr EnumFlags& operator|=(Enum e) const noexcept
            {
                flags |= Underlying(e);
                return *this;
            }

        	constexpr EnumFlags operator&(Enum e) const noexcept
            {
                return EnumFlags{ flags | Underlying(e) };
            }

        	constexpr EnumFlags& operator&=(Enum e) const noexcept
            {
                flags &= Underlying(e);
                return *this;
            }

        	constexpr EnumFlags operator&(EnumFlags o) const noexcept
            {
                return EnumFlags{ flags | o.flags };
            }

        	constexpr EnumFlags& operator&=(EnumFlags o) const noexcept
            {
                flags &= o.flags;
                return *this;
            }
        	
            Flags flags{};
        };

    	template<typename T, typename ... Ts>
    	struct AreDiff
    	{
            static constexpr bool value = (!std::is_same_v<T, Ts> && ...) && AreDiff<Ts...>::value;
    	};

    	template<typename T>
    	struct AreDiff<T>
    	{
            static constexpr bool value = true;
    	};

    	template<typename T, typename ... Ts>
    	struct OneOf
    	{
            static constexpr bool value = (std::is_same_v<T, Ts> || ...);
    	};

        template<typename T>
    	struct ValueWrapper
        {
            constexpr ValueWrapper(T& ref) : ref_(ref)
            {
            }
        	
            constexpr ValueWrapper& operator=(T value) noexcept
            {

                ref_ = value;
                return *this;
			}
        	
        private:
            T& ref_;
        };

    	
    	
    	template<typename T>
		constexpr T clamp(const T value, const T min, const T max) noexcept
    	{
            return value > min ? (value < max ? value : max) : min;
    	}
    	
    } // namespace util
    
    
} // namespace mvk






#endif