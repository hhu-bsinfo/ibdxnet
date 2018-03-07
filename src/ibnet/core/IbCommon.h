/*
 * Copyright (C) 2018 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef IBNET_CORE_IBCOMMON_H
#define IBNET_CORE_IBCOMMON_H

namespace ibnet {
namespace core {

/**
 * Some common stuff for the core namespace
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.02.2018
 */

/**
 * Translate libverb's work completion status codes to readable strings
 */
static const char* WORK_COMPLETION_STATUS_CODE[] = {
        "IBV_WC_SUCCESS",
        "IBV_WC_LOC_LEN_ERR",
        "IBV_WC_LOC_QP_OP_ERR",
        "IBV_WC_LOC_EEC_OP_ERR",
        "IBV_WC_LOC_PROT_ERR",
        "IBV_WC_WR_FLUSH_ERR",
        "IBV_WC_MW_BIND_ERR",
        "IBV_WC_BAD_RESP_ERR",
        "IBV_WC_LOC_ACCESS_ERR",
        "IBV_WC_REM_INV_REQ_ERR",
        "IBV_WC_REM_ACCESS_ERR",
        "IBV_WC_REM_OP_ERR",
        "IBV_WC_RETRY_EXC_ERR",
        "IBV_WC_RNR_RETRY_EXC_ERR",
        "IBV_WC_LOC_RDD_VIOL_ERR",
        "IBV_WC_REM_INV_RD_REQ_ERR",
        "IBV_WC_REM_ABORT_ERR",
        "IBV_WC_INV_EECN_ERR",
        "IBV_WC_INV_EEC_STATE_ERR",
        "IBV_WC_FATAL_ERR",
        "IBV_WC_RESP_TIMEOUT_ERR",
        "IBV_WC_GENERAL_ERR"
};

}
}

#endif //IBNET_CORE_IBCOMMON_H
