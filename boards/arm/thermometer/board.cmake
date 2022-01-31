# Copyright (c) 2019 Nordic Semiconductor ASA

# SPDX-License-Identifier: Apache-2.0

set(OPENOCD_NRF5_SUBFAMILY ON)

board_runner_args(jlink "--device=nrf52" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd-nrf5.board.cmake)
