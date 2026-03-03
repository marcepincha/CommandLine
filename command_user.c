/**
 * \file command_user.c
 * \brief User-defined CLI command table and handlers.
 *
 * This file contains the command table definition and all user-specific
 * command handlers. Users should extend this file to add their own commands.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.0
 */

#include "command_user.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ads1299lib.h"

/**
 * \brief Example AT command handler.
 *
 * This is a minimal example handler. Replace with your command logic.
 *
 * \param[in] arg Command argument string (may be NULL)
 */
void ATexec(const char *arg);
void cmd_start(const char *arg);
void cmd_stop(const char *arg);
void cmd_dr(const char *arg);
void cmd_chmod(const char *arg);
void cmd_gain(const char *arg);
void cmd_dr_read(const char *arg);
void cmd_chmod_read(const char *arg);
void cmd_gain_read(const char *arg);

/**
 * \brief Command table definition.
 *
 * Each command defines its ID and optional handler pointers for:
 * - write: Called with "command=arg" format
 * - read:  Called with "command?" format
 * - excecute: Called with "command arg" or "command" format
 *
 * Set handler to NULL if operation is not supported.
 */
const cli_cmd_t cli_Table[] = {
    {"AT", NULL, NULL, ATexec},
    {"START", NULL, NULL, cmd_start},
    {"STOP", NULL, NULL, cmd_stop},
    {"DR", cmd_dr, cmd_dr_read, NULL},
    {"CHMOD", cmd_chmod, cmd_chmod_read, NULL},
    {"GAIN", cmd_gain, cmd_gain_read, NULL},
};

/** \brief Size of the command table */
const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

/**
 * \brief Example AT command execution.
 *
 * Demonstrates a simple command handler.
 *
 * \param[in] arg Command argument (unused in this example)
 */
void ATexec(const char *arg) { CMD_Print("OK\r\n"); }

extern ads_t ads_device;
extern ads_init_t init_config;

void ads_interface_spi_rx_sample_DMA_prepare_next(ads_t *self, uint8_t *buff);
extern volatile uint8_t buffer0[];

void cmd_start(const char *arg) {
  ads_start(&ads_device);
  ads_interface_spi_rx_sample_DMA_prepare_next(&ads_device, (uint8_t *)buffer0);
  CMD_Print("ADS1299 Started\r\n");
}

void cmd_stop(const char *arg) {
  ads_stop(&ads_device);
  CMD_Print("ADS1299 Stopped\r\n");
}

void cmd_dr(const char *arg) {
  if (arg != NULL) {
    int dr = atoi(arg);
    if (dr >= ADS_DR_16KSPS && dr <= ADS_DR_250SPS) {
      ads_set_data_rate(&ads_device, (ads_datarate_t)dr);
      CMD_Print("Data rate updated\r\n");
    } else {
      CMD_Print("Invalid Data Rate\r\n");
    }
  }
}

void cmd_chmod(const char *arg) {
  if (arg != NULL) {
    int ch, mode;
    if (sscanf(arg, "%d,%d", &ch, &mode) == 2) {
      if (ch >= 0 && ch < ads_device.num_channels && mode >= ADS_CHMOD_NORMAL &&
          mode <= ADS_CHMOD_BIASN) {
        ads_channel_mode_t modes[ADS_MAX_CHANNELS];
        ads_get_ch_mode(&ads_device, modes);
        modes[ch] = (ads_channel_mode_t)mode;
        ads_set_ch_mode(&ads_device, modes);
        CMD_Print("Channel mode updated\r\n");
      } else {
        CMD_Print("Invalid channel or mode\r\n");
      }
    } else {
      CMD_Print("Usage: CHMOD=ch,mode\r\n");
    }
  }
}

void cmd_gain(const char *arg) {
  if (arg != NULL) {
    int ch, gain;
    if (sscanf(arg, "%d,%d", &ch, &gain) == 2) {
      if (ch >= 0 && ch < ads_device.num_channels && gain >= ADS_GAIN_1 &&
          gain <= ADS_GAIN_24) {
        ads_gain_t gains[ADS_MAX_CHANNELS];
        ads_get_ch_gain(&ads_device, gains);
        gains[ch] = (ads_gain_t)gain;
        ads_set_ch_gain(&ads_device, gains);
        CMD_Print("Channel gain updated\r\n");
      } else {
        CMD_Print("Invalid channel or gain\r\n");
      }
    } else {
      CMD_Print("Usage: GAIN=ch,gain\r\n");
    }
  }
}

void cmd_dr_read(const char *arg) {
  ads_datarate_t dr = ads_get_data_rate(&ads_device);
  char buf[32];
  snprintf(buf, sizeof(buf), "DR=%d\r\n", dr);
  CMD_Print(buf);
}

void cmd_chmod_read(const char *arg) {
  ads_channel_mode_t modes[ADS_MAX_CHANNELS];
  ads_get_ch_mode(&ads_device, modes);
  char buf[64];
  for (int i = 0; i < ads_device.num_channels; i++) {
    snprintf(buf, sizeof(buf), "CHMOD[%d]=%d\r\n", i, modes[i]);
    CMD_Print(buf);
  }
}

void cmd_gain_read(const char *arg) {
  ads_gain_t gains[ADS_MAX_CHANNELS];
  ads_get_ch_gain(&ads_device, gains);
  char buf[64];
  for (int i = 0; i < ads_device.num_channels; i++) {
    snprintf(buf, sizeof(buf), "GAIN[%d]=%d\r\n", i, gains[i]);
    CMD_Print(buf);
  }
}
