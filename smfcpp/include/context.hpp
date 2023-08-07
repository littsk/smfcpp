#ifndef SMFCPP__CONTEXT_HPP_
#define SMFCPP__CONTEXT_HPP_

#include <memory>

#include "macros.hpp"


namespace smfcpp{

class Context: public std::enable_shared_from_this<Context>
{
public:
    SMFCPP_SHARED_PTR_DEFINITIONS(Context)
    SMFCPP_DISABLE_COPY(Context)

    explicit Context() = default;
};

}

#endif