#ifndef SMFCPP__MACROS_HPP_
#define SMFCPP__MACROS_HPP_


#include <memory>
/**
 * Defines aliases and static functions for using the Class with smart pointers.
 * If you're declaring a new class, consider using the \
 * SMFCPP_SMART_PTR_DEFINITIONS(ClassName) macro.
 * It'll help streamline your use of smart pointers whithin that class.
**/
#define SMFCPP_DISABLE_COPY(...) \
    __VA_ARGS__(const __VA_ARGS__ &) = delete; \
    __VA_ARGS__ & operator=(const __VA_ARGS__ &) = delete; 

#define SMFCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(...) \
    SMFCPP_SHARED_PTR_DEFINITIONS(__VA_ARGS__) \
    SMFCPP_WEAK_PTR_DEFINITIONS(__VA_ARGS__) \
    __SMFCPP_UNIQUE_PTR_ALIAS(__VA_ARGS__)

#define SMFCPP_SMART_PTR_DEFINITIONS(...) \
    SMFCPP_SHARED_PTR_DEFINITIONS(__VA_ARGS__) \
    SMFCPP_WEAK_PTR_DEFINITIONS(__VA_ARGS__) \
    SMFCPP_UNIQUE_PTR_DEFINITIONS(__VA_ARGS__) 

#define SMFCPP_SMART_PTR_ALIASES_ONLY(...) \
    __SMFCPP_SHARED_PTR_ALIAS(__VA_ARGS__) \
    __SMFCPP_WEAK_PTR_ALIAS(__VA_ARGS__) \
    __SMFCPP_UNIQUE_PTR_ALIAS(__VA_ARGS__) \
    __SMFCPP_MAKE_SHARED_DEFINITION(__VA_ARGS__)

#define SMFCPP_SHARED_PTR_DEFINITIONS(...) \
    __SMFCPP_SHARED_PTR_ALIAS(__VA_ARGS__) \
    __SMFCPP_MAKE_SHARED_DEFINITION(__VA_ARGS__)

#define __SMFCPP_SHARED_PTR_ALIAS(...) \
    using SharedPtr = std::shared_ptr<__VA_ARGS__>; \
    using ConstSharedPtr = std::shared_ptr<const __VA_ARGS__>;

// using std::make_shared to get better performence
#define __SMFCPP_MAKE_SHARED_DEFINITION(...) \
    template<typename... Args> \
    static std::shared_ptr<__VA_ARGS__> \
    make_shared(Args && ... args) \
    { \
        return std::make_shared<__VA_ARGS__>(std::forward<Args>(args)...); \
    }

#define SMFCPP_WEAK_PTR_DEFINITIONS(...) \
    __SMFCPP_WEAK_PTR_ALIAS(__VA_ARGS__)

#define __SMFCPP_WEAK_PTR_ALIAS(...) \
    using WeakPtr = std::weak_ptr<__VA_ARGS__>; \
    using ConstWeakPtr = std::weak_ptr<const __VA_ARGS__>;

#define SMFCPP_UNIQUE_PTR_DEFINITIONS(...) \
    __SMFCPP_UNIQUE_PTR_ALIAS(__VA_ARGS__) \
    __SMFCPP_MAKE_UNIQUE_DEFINITION(__VA_ARGS__)

#define __SMFCPP_UNIQUE_PTR_ALIAS(...) \
    using UniquePtr = std::unique_ptr<__VA_ARGS__>;

#define __SMFCPP_MAKE_UNIQUE_DEFINITION(...) \
    template<typename... Args> \
    std::unique_ptr<__VA_ARGS__> \
    make_unique(Args && ... args) \
    { \
        return std::make_unique<__VA_ARGS__>(std::forward<Args>(args)...); \
    }





#endif