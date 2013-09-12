#ifndef UNET_USER_CONFIG_HPP
#define UNET_USER_CONFIG_HPP

// ----=====================================================================----
//     General settings
// ----=====================================================================----

// If this macro is defined, the user has to provide the throw_exception()
// function. The function's signature is
// void ::uNet::throw_exception(const std::exception& e);
// This function must never return.
// #define UNET_CUSTOM_THROW_EXCEPTION

// If this macro is defined, assertions are enabled in the WEOS library.
// By default, the assertion is checked using the assert() function from
// <cassert>.
// #define UNET_ENABLE_ASSERT

// If this macro is defined, the user has to provide a handler for a failed
// assertion. The function's signature is
// void ::uNet::assert_failed(const char* condition, const char* function,
//                            const char* file, int line);
// Note: If UNET_ENABLE_ASSERT is not defined, this macro has no effect.
// #define UNET_CUSTOM_ASSERT_HANDLER

#endif // UNET_USER_CONFIG_HPP
