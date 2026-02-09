# SPDX-License-Identifier: BSD-2-Clause

# MP13 boards
flavor_dts_file-135F_DK_OSTL = stm32mp135f-dk-ostl.dts
flavor_dts_file-135F_DK_PMIC1LD_OSTL = stm32mp135f-dk-pmic1ld-ostl.dts

flavorlist-MP13 += $(flavor_dts_file-135F_DK_OSTL)
flavorlist-MP13 += $(flavor_dts_file-135F_DK_PMIC1LD_OSTL)

# MP15 boards
flavor_dts_file-157F_DK2_OSTL = stm32mp157f-dk2-ostl.dts
flavor_dts_file-157F_EV1_OSTL = stm32mp157f-ev1-ostl.dts

flavorlist-MP15 += $(flavor_dts_file-157F_DK2_OSTL)
flavorlist-MP15 += $(flavor_dts_file-157F_EV1_OSTL)

flavorlist-cryp-512M += $(flavor_dts_file-135F_DK_OSTL)
flavorlist-cryp-512M += $(flavor_dts_file-135F_DK_PMIC1LD_OSTL)
flavorlist-cryp-512M += $(flavor_dts_file-157F_DK2_OSTL)
flavorlist-cryp-1G += $(flavor_dts_file-157F_EV1_OSTL)

flavorlist-MP1-PWR-OSTL += $(flavor_dts_file-135F_DK_OSTL)
flavorlist-MP1-PWR-OSTL += $(flavor_dts_file-157F_DK2_OSTL)
flavorlist-MP1-PWR-OSTL += $(flavor_dts_file-157F_EV1_OSTL)
