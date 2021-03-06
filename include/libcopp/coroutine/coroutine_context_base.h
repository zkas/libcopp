#ifndef _COPP_COROUTINE_CONTEXT_COROUTINE_CONTEXT_BASE_H_
#define _COPP_COROUTINE_CONTEXT_COROUTINE_CONTEXT_BASE_H_


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstddef>

#include <libcopp/coroutine/coroutine_runnable_base.h>
#include <libcopp/fcontext/all.hpp>
#include <libcopp/stack/stack_context.h>
#include <libcopp/utils/atomic_int_type.h>
#include <libcopp/utils/features.h>
#include <libcopp/utils/non_copyable.h>

#ifdef COPP_MACRO_USE_SEGMENTED_STACKS
#define COROUTINE_CONTEXT_BASE_USING_BASE_SEGMENTED_STACKS(base_type) using base_type::caller_stack_;
#else
#define COROUTINE_CONTEXT_BASE_USING_BASE_SEGMENTED_STACKS(base_type)
#endif

#define COROUTINE_CONTEXT_BASE_USING_BASE(base_type) \
protected:                                           \
    using base_type::caller_;                        \
    using base_type::callee_;                        \
    using base_type::callee_stack_;                  \
    COROUTINE_CONTEXT_BASE_USING_BASE_SEGMENTED_STACKS(base_type)

namespace copp {
    namespace detail {
        /**
         * @brief base type of all coroutine context
         */
        class coroutine_context_base : utils::non_copyable {
        public:
            /**
            * @brief status of safe coroutine context base
            */
            struct status_t {
                enum type {
                    EN_CRS_INVALID = 0, //!< EN_CRS_INVALID
                    EN_CRS_READY,       //!< EN_CRS_READY
                    EN_CRS_RUNNING,     //!< EN_CRS_RUNNING
                    EN_CRS_FINISHED,    //!< EN_CRS_FINISHED
                    EN_CRS_EXITED,      //!< EN_CRS_EXITED
                };
            };

        private:
            int runner_ret_code_;             /** coroutine return code **/
            coroutine_runnable_base *runner_; /** coroutine runner **/
            void *priv_data_;
            util::lock::atomic_int_type<int> status_; /** status **/

            struct jump_src_data_t {
                coroutine_context_base *from_co;
                coroutine_context_base *to_co;
                void *priv_data;
            };

        protected:
            fcontext::fcontext_t caller_; /** caller runtime context **/
            fcontext::fcontext_t callee_; /** callee runtime context **/

            stack_context callee_stack_; /** callee stack context **/
#ifdef COPP_MACRO_USE_SEGMENTED_STACKS
            stack_context caller_stack_; /** caller stack context **/
#endif

        public:
            coroutine_context_base() UTIL_CONFIG_NOEXCEPT;
            virtual ~coroutine_context_base();

        public:
            /**
             * @brief create coroutine context at stack context callee_
             * @param runner runner
             * @param func fcontext callback
             * @return COPP_EC_SUCCESS or error code
             */
            virtual int
            create(coroutine_runnable_base *runner,
                   void (*func)(::copp::fcontext::transfer_t) = &coroutine_context_base::coroutine_context_callback) UTIL_CONFIG_NOEXCEPT;

            /**
             * @brief start coroutine
             * @param priv_data private data, will be passed to runner operator() or return to yield
             * @return COPP_EC_SUCCESS or error code
             */
            virtual int start(void *priv_data = UTIL_CONFIG_NULLPTR);

            /**
             * @brief yield coroutine
             * @param priv_data private data, if not NULL, will get the value from start(priv_data) or resume(priv_data)
             * @return COPP_EC_SUCCESS or error code
             */
            virtual int yield(void **priv_data = UTIL_CONFIG_NULLPTR);

            /**
             * @brief resume coroutine
             * @param priv_data private data, will be passed to runner operator() or return to yield
             * @return COPP_EC_SUCCESS or error code
             */
            virtual int resume(void *priv_data = UTIL_CONFIG_NULLPTR);

        protected:
            /**
             * @brief coroutine entrance function
             */
            inline void run_and_recv_retcode(void *priv_data) {
                if (UTIL_CONFIG_NULLPTR == runner_) return;

                runner_ret_code_ = (*runner_)(priv_data);
            }

        protected:
            /**
             * @brief set runner
             * @param runner
             * @return COPP_EC_SUCCESS or error code
             */
            virtual int set_runner(coroutine_runnable_base *runner) UTIL_CONFIG_NOEXCEPT;

        public:
            /**
             * get runner of this coroutine context
             * @return NULL of pointer of runner
             */
            inline coroutine_runnable_base *get_runner() UTIL_CONFIG_NOEXCEPT { return runner_; }

            /**
             * get runner of this coroutine context (const)
             * @return NULL of pointer of runner
             */
            inline const coroutine_runnable_base *get_runner() const UTIL_CONFIG_NOEXCEPT { return runner_; }

            /**
             * @brief get runner return code
             * @return
             */
            inline int get_ret_code() const UTIL_CONFIG_NOEXCEPT { return runner_ret_code_; }

            /**
             * @brief get runner return code
             * @return true if coroutine has run and finished
             */
            bool is_finished() const UTIL_CONFIG_NOEXCEPT;

            /**
             * @brief set private data(raw pointer)
             * @note cotask set this private data to pointer of cotask, if you use cotask, do not use this function
             */
            inline void set_private_data(void *ptr) UTIL_CONFIG_NOEXCEPT { priv_data_ = ptr; }

            /**
             * @brief get private data(raw pointer)
             * @note cotask set this private data to pointer of cotask, if you use cotask, do not use this function
             */
            inline void *get_private_data() const UTIL_CONFIG_NOEXCEPT { return priv_data_; }

        protected:
            /**
             * @brief call platform jump to asm instruction
             * @param to_fctx jump to function context
             * @param from_sctx jump from stack context(only used for save segment stack)
             * @param to_sctx jump to stack context(only used for set segment stack)
             * @param jump_transfer jump data
             */
            static void jump_to(fcontext::fcontext_t &to_fctx, stack_context &from_sctx, stack_context &to_sctx,
                                jump_src_data_t &jump_transfer) UTIL_CONFIG_NOEXCEPT;

            /**
             * @brief fcontext entrance function
             * @param src_ctx where jump from
             */
            static void coroutine_context_callback(::copp::fcontext::transfer_t src_ctx);
        };
    }

    namespace this_coroutine {
        /**
         * @brief get current coroutine
         * @see detail::coroutine_context_base
         * @return pointer of current coroutine, if not in coroutine, return NULL
         */
        detail::coroutine_context_base *get_coroutine() UTIL_CONFIG_NOEXCEPT;

        /**
         * @brief get current coroutine and try to convert type
         * @see get_coroutine
         * @see detail::coroutine_context_base
         * @return pointer of current coroutine, if not in coroutine or fail to convert type, return NULL
         */
        template <typename Tc>
        Tc *get() {
            return dynamic_cast<Tc *>(get_coroutine());
        }

        /**
         * @brief yield current coroutine
         * @param priv_data private data, if not NULL, will get the value from start(priv_data) or resume(priv_data)
         * @return 0 or error code
         */
        int yield(void **priv_data = UTIL_CONFIG_NULLPTR);
    }
}

#endif
