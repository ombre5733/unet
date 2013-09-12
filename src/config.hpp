#ifndef UNET_CONFIG_HPP
#define UNET_CONFIG_HPP

#if !defined(UNET_USER_CONFIG)
//#  error "The location of the user config has not been defined."
#define UNET_USER_CONFIG   "user_config.hpp"
#endif
#include UNET_USER_CONFIG

#if !defined(UNET_CUSTOM_THROW_EXCEPTION)
    namespace uNet
    {
        template <typename ExceptionT>
        /*BOOST_ATTRIBUTE_NORETURN*/ inline void throw_exception(const ExceptionT& e)
        {
            throw e;
        }
    } // namespace uNet
#else
#   include <exception>
    namespace uNet
    {
        // This is only a declaration - the definition has to be provided by the user.
        void throw_exception(const std::exception& e);
     } // namespace uNet
#endif


#if defined(UNET_ENABLE_ASSERT)
#  if defined(UNET_CUSTOM_ASSERT_HANDLER)
     namespace uNet
     {
         void assert_failed(const char* condition, const char* function,
                            const char* file, int line);
     } // namespace uNet
#    define UNET_ASSERT(cond)                                                  \
         do { if (!(cond)) ::uNet::assert_failed(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__) } while(0)
#  else
#    include <cassert>
#    define UNET_ASSERT(cond)   assert(cond)
#  endif // UNET_CUSTOM_ASSERT_HANDLER
#else
#  define UNET_ASSERT(cond)   ((void)0)
#endif // UNET_ENABLE_ASSERT

#endif // UNET_CONFIG_HPP
