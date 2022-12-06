/**
 * @file ucanopen_server.cpp
 * @ingroup ucanopen
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "ucanopen_server.h"


namespace ucanopen {


unsigned char impl::cana_rpdoinfo_dualcore_alloc[sizeof(emb::Array<impl::RpdoInfo, 4>)]
						 __attribute__((section("shared_ucanopen_cana_rpdo_data"), retain));
unsigned char impl::canb_rpdoinfo_dualcore_alloc[sizeof(emb::Array<impl::RpdoInfo, 4>)]
						 __attribute__((section("shared_ucanopen_canb_rpdo_data"), retain));


} // namespace ucanopen


