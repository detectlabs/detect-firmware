#ifndef MACROS_H__
#define MACROS_H__


/** @brief Check if the error code is equal to NRF_SUCCESS. If it is not, return the error code.
 */
#define RETURN_IF_ERROR(PARAM)                                                                     \
    if ((PARAM) != NRF_SUCCESS)                                                                    \
    {                                                                                              \
        return (PARAM);                                                                            \
    }


#endif