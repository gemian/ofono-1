/*
 *
 *  RIL constants for MTK modem
 *
 *  Copyright (C) 2016 Canonical Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef MTK2_CONSTANTS_H
#define MTK2_CONSTANTS_H

/* RIL Request Messages */
#define MTK2_RIL_REQUEST_MODEM_POWEROFF 2010
#define MTK2_RIL_REQUEST_DUAL_SIM_MODE_SWITCH 2011
#define MTK2_RIL_REQUEST_SET_GPRS_CONNECT_TYPE 2015
#define MTK2_RIL_REQUEST_SET_GPRS_TRANSFER_TYPE 2016
#define MTK2_RIL_REQUEST_MODEM_POWERON 2028
#define MTK2_RIL_REQUEST_SET_CALL_INDICATION 2086
#define MTK2_RIL_REQUEST_GET_PHONE_CAPABILITY 2030
#define MTK2_RIL_REQUEST_SET_PHONE_CAPABILITY 2031
#define MTK2_RIL_REQUEST_SET_TRM 2043
#define MTK2_RIL_REQUEST_SET_FD_MODE 2061
#define MTK2_RIL_REQUEST_RESUME_REGISTRATION 2065
#define MTK2_RIL_REQUEST_STORE_MODEM_TYPE 2066
#define MTK2_RIL_REQUEST_QUERY_MODEM_TYPE 2067
#define MTK2_RIL_REQUEST_SET_DATA_ON_TO_MD 2111

/* RIL Unsolicited Messages */
#define MTK2_RIL_UNSOL_MTK_BASE 3000

#define MTK2_RIL_UNSOL_NEIGHBORING_CELL_INFO (MTK2_RIL_UNSOL_MTK_BASE + 0)
#define MTK2_RIL_UNSOL_NETWORK_INFO (MTK2_RIL_UNSOL_MTK_BASE + 1)
#define MTK2_RIL_UNSOL_PHB_READY_NOTIFICATION (MTK2_RIL_UNSOL_MTK_BASE + 2)
#define MTK2_RIL_UNSOL_SIM_INSERTED_STATUS (MTK2_RIL_UNSOL_MTK_BASE + 3)
#define MTK2_RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE (MTK2_RIL_UNSOL_MTK_BASE + 4)
#define MTK2_RIL_UNSOL_ME_SMS_STORAGE_FULL (MTK2_RIL_UNSOL_MTK_BASE + 5)
#define MTK2_RIL_UNSOL_SMS_READY_NOTIFICATION (MTK2_RIL_UNSOL_MTK_BASE + 6)
#define MTK2_RIL_UNSOL_SCRI_RESULT (MTK2_RIL_UNSOL_MTK_BASE + 7)
#define MTK2_RIL_UNSOL_SIM_MISSING (MTK2_RIL_UNSOL_MTK_BASE + 8)
#define MTK2_RIL_UNSOL_GPRS_DETACH (MTK2_RIL_UNSOL_MTK_BASE + 9)
#define MTK2_RIL_UNSOL_ATCI_RESPONSE (MTK2_RIL_UNSOL_MTK_BASE + 10)
#define MTK2_RIL_UNSOL_SIM_RECOVERY (MTK2_RIL_UNSOL_MTK_BASE + 11)
#define MTK2_RIL_UNSOL_VIRTUAL_SIM_ON (MTK2_RIL_UNSOL_MTK_BASE + 12)
#define MTK2_RIL_UNSOL_VIRTUAL_SIM_OFF (MTK2_RIL_UNSOL_MTK_BASE + 13)
#define MTK2_RIL_UNSOL_INVALID_SIM (MTK2_RIL_UNSOL_MTK_BASE + 14)
#define MTK2_RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 15)
#define MTK2_RIL_UNSOL_RESPONSE_ACMT (MTK2_RIL_UNSOL_MTK_BASE + 16)
#define MTK2_RIL_UNSOL_EF_CSP_PLMN_MODE_BIT (MTK2_RIL_UNSOL_MTK_BASE + 17)
#define MTK2_RIL_UNSOL_IMEI_LOCK (MTK2_RIL_UNSOL_MTK_BASE + 18)
#define MTK2_RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 19)
#define MTK2_RIL_UNSOL_SIM_PLUG_OUT (MTK2_RIL_UNSOL_MTK_BASE + 20)
#define MTK2_RIL_UNSOL_SIM_PLUG_IN (MTK2_RIL_UNSOL_MTK_BASE + 21)
#define MTK2_RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION (MTK2_RIL_UNSOL_MTK_BASE + 22)
#define MTK2_RIL_UNSOL_RESPONSE_PLMN_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 23)
#define MTK2_RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED (MTK2_RIL_UNSOL_MTK_BASE + 24)
#define MTK2_RIL_UNSOL_STK_EVDL_CALL (MTK2_RIL_UNSOL_MTK_BASE + 25)
#define MTK2_RIL_UNSOL_DATA_PACKETS_FLUSH (MTK2_RIL_UNSOL_MTK_BASE + 26)
#define MTK2_RIL_UNSOL_FEMTOCELL_INFO (MTK2_RIL_UNSOL_MTK_BASE + 27)
#define MTK2_RIL_UNSOL_STK_SETUP_MENU_RESET (MTK2_RIL_UNSOL_MTK_BASE + 28)
#define MTK2_RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 29)
#define MTK2_RIL_UNSOL_ECONF_SRVCC_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 30)
#define MTK2_RIL_UNSOL_IMS_ENABLE_DONE (MTK2_RIL_UNSOL_MTK_BASE + 31)
#define MTK2_RIL_UNSOL_IMS_DISABLE_DONE (MTK2_RIL_UNSOL_MTK_BASE + 32)
#define MTK2_RIL_UNSOL_IMS_REGISTRATION_INFO (MTK2_RIL_UNSOL_MTK_BASE + 33)
#define MTK2_RIL_UNSOL_DEDICATE_BEARER_ACTIVATED (MTK2_RIL_UNSOL_MTK_BASE + 34)
#define MTK2_RIL_UNSOL_DEDICATE_BEARER_MODIFIED (MTK2_RIL_UNSOL_MTK_BASE + 35)
#define MTK2_RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED (MTK2_RIL_UNSOL_MTK_BASE + 36)
#define MTK2_RIL_UNSOL_RAC_UPDATE (MTK2_RIL_UNSOL_MTK_BASE + 37)
#define MTK2_RIL_UNSOL_ECONF_RESULT_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 38)
#define MTK2_RIL_UNSOL_MELOCK_NOTIFICATION (MTK2_RIL_UNSOL_MTK_BASE + 39)
#define MTK2_RIL_UNSOL_CALL_FORWARDING (MTK2_RIL_UNSOL_MTK_BASE + 40)
#define MTK2_RIL_UNSOL_CRSS_NOTIFICATION (MTK2_RIL_UNSOL_MTK_BASE + 41)
#define MTK2_RIL_UNSOL_INCOMING_CALL_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 42)
#define MTK2_RIL_UNSOL_CIPHER_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 43)
#define MTK2_RIL_UNSOL_CNAP (MTK2_RIL_UNSOL_MTK_BASE + 44)
#define MTK2_RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 45)
#define MTK2_RIL_UNSOL_DATA_ALLOWED (MTK2_RIL_UNSOL_MTK_BASE + 46)
#define MTK2_RIL_UNSOL_STK_CALL_CTRL (MTK2_RIL_UNSOL_MTK_BASE + 47)
#define MTK2_RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_SUPPORT (MTK2_RIL_UNSOL_MTK_BASE + 48)
#define MTK2_RIL_UNSOL_CALL_INFO_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 49)
#define MTK2_RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO (MTK2_RIL_UNSOL_MTK_BASE + 50)
#define MTK2_RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 51)
#define MTK2_RIL_UNSOL_SPEECH_CODEC_INFO (MTK2_RIL_UNSOL_MTK_BASE + 52)
#define MTK2_RIL_UNSOL_MD_STATE_CHANGE (MTK2_RIL_UNSOL_MTK_BASE + 53)
#define MTK2_RIL_UNSOL_REMOVE_RESTRICT_EUTRAN (MTK2_RIL_UNSOL_MTK_BASE + 54)
#define MTK2_RIL_UNSOL_MO_DATA_BARRING_INFO (MTK2_RIL_UNSOL_MTK_BASE + 55)
#define MTK2_RIL_UNSOL_SSAC_BARRING_INFO (MTK2_RIL_UNSOL_MTK_BASE + 56)
#define MTK2_RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR (MTK2_RIL_UNSOL_MTK_BASE + 57)
#define MTK2_RIL_UNSOL_ABNORMAL_EVENT (MTK2_RIL_UNSOL_MTK_BASE + 58)
#define MTK2_RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY (MTK2_RIL_UNSOL_MTK_BASE + 59)
#define MTK2_RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE (MTK2_RIL_UNSOL_MTK_BASE + 60)
#define MTK2_RIL_UNSOL_LTE_BG_SEARCH_STATUS (MTK2_RIL_UNSOL_MTK_BASE + 61)
#define MTK2_RIL_UNSOL_GMSS_RAT_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 62)
#define MTK2_RIL_UNSOL_CDMA_CARD_TYPE (MTK2_RIL_UNSOL_MTK_BASE + 63)
#define MTK2_RIL_UNSOL_IMS_ENABLE_START (MTK2_RIL_UNSOL_MTK_BASE + 64)
#define MTK2_RIL_UNSOL_IMS_DISABLE_START (MTK2_RIL_UNSOL_MTK_BASE + 65)
#define MTK2_RIL_UNSOL_IMSI_REFRESH_DONE (MTK2_RIL_UNSOL_MTK_BASE + 66)
#define MTK2_RIL_UNSOL_EUSIM_READY (MTK2_RIL_UNSOL_MTK_BASE + 67)
#define MTK2_RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND (MTK2_RIL_UNSOL_MTK_BASE + 68)
#define MTK2_RIL_UNSOL_WORLD_MODE_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 69)
#define MTK2_RIL_UNSOL_VT_STATUS_INFO (MTK2_RIL_UNSOL_MTK_BASE + 70)
#define MTK2_RIL_UNSOL_VT_RING_INFO (MTK2_RIL_UNSOL_MTK_BASE + 71)
#define MTK2_RIL_UNSOL_VSIM_OPERATION_INDICATION (MTK2_RIL_UNSOL_MTK_BASE + 72)
#define MTK2_RIL_UNSOL_SET_ATTACH_APN (MTK2_RIL_UNSOL_MTK_BASE + 73)
#define MTK2_RIL_UNSOL_MAL_AT_INFO (MTK2_RIL_UNSOL_MTK_BASE + 74)
#define MTK2_RIL_UNSOL_MAIN_SIM_INFO (MTK2_RIL_UNSOL_MTK_BASE + 75)
#define MTK2_RIL_UNSOL_TRAY_PLUG_IN (MTK2_RIL_UNSOL_MTK_BASE + 76)
#define MTK2_RIL_UNSOL_CALLMOD_CHANGE_INDICATOR (MTK2_RIL_UNSOL_MTK_BASE + 77)
#define MTK2_RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR (MTK2_RIL_UNSOL_MTK_BASE + 78)
#define MTK2_RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE (MTK2_RIL_UNSOL_MTK_BASE + 79)
#define MTK2_RIL_UNSOL_SETUP_DATA_CALL_RESPONSE (MTK2_RIL_UNSOL_MTK_BASE + 80)
#define MTK2_RIL_UNSOL_ON_USSI (MTK2_RIL_UNSOL_MTK_BASE + 81)
#define MTK2_RIL_UNSOL_CDMA_CALL_ACCEPTED (MTK2_RIL_UNSOL_MTK_BASE + 82)
#define MTK2_RIL_UNSOL_UTK_SESSION_END (MTK2_RIL_UNSOL_MTK_BASE + 83)
#define MTK2_RIL_UNSOL_UTK_PROACTIVE_COMMAND (MTK2_RIL_UNSOL_MTK_BASE + 84)
#define MTK2_RIL_UNSOL_UTK_EVENT_NOTIFY (MTK2_RIL_UNSOL_MTK_BASE + 85)
#define MTK2_RIL_UNSOL_VIA_GPS_EVENT (MTK2_RIL_UNSOL_MTK_BASE + 86)
#define MTK2_RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE (MTK2_RIL_UNSOL_MTK_BASE + 87)
#define MTK2_RIL_UNSOL_CDMA_PLMN_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 88)
#define MTK2_RIL_UNSOL_VIA_INVALID_SIM_DETECTED (MTK2_RIL_UNSOL_MTK_BASE + 89)
#define MTK2_RIL_UNSOL_ENG_MODE_NETWORK_INFO (MTK2_RIL_UNSOL_MTK_BASE + 90)
#define MTK2_RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 91)
#define MTK2_RIL_UNSOL_LTE_EARFCN_INFO (MTK2_RIL_UNSOL_MTK_BASE + 92)
#define MTK2_RIL_UNSOL_CDMA_IMSI_READY (MTK2_RIL_UNSOL_MTK_BASE + 93)
#define MTK2_RIL_UNSOL_CDMA_SIGNAL_FADE (MTK2_RIL_UNSOL_MTK_BASE + 94)
#define MTK2_RIL_UNSOL_CDMA_TONE_SIGNALS (MTK2_RIL_UNSOL_MTK_BASE + 95)
#define MTK2_RIL_UNSOL_NETWORK_EXIST (MTK2_RIL_UNSOL_MTK_BASE + 96)
#define MTK2_RIL_UNSOL_MODULATION_INFO (MTK2_RIL_UNSOL_MTK_BASE + 97)
#define MTK2_RIL_UNSOL_NETWORK_EVENT (MTK2_RIL_UNSOL_MTK_BASE + 98)
#define MTK2_RIL_UNSOL_MBIM_RESPONSE (MTK2_RIL_UNSOL_MTK_BASE + 99)
#define MTK2_RIL_UNSOL_MAL_DATA_CALL_LIST_CHANGED (MTK2_RIL_UNSOL_MTK_BASE + 100)
#define MTK2_RIL_UNSOL_EMBMS_AT_INFO  (MTK2_RIL_UNSOL_MTK_BASE + 101)
#define MTK2_RIL_UNSOL_RADIO_CAPABILITY_SWITCH_START (MTK2_RIL_UNSOL_MTK_BASE + 102)

#endif /* MTK2_CONSTANTS_H */