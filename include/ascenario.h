/*
*  ALSA Scenario header file
*
*   This library is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as
*   published by the Free Software Foundation; either version 2.1 of
*   the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public
*   License along with this library; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*
*  Copyright (C) 2008-2009 SlimLogic Ltd
*  Authors: Liam Girdwood <lrg@slimlogic.co.uk>
*           Stefan Schmidt <stefan@slimlogic.co.uk>
*/

/**
 * Scenario IDs
 *
 * Standard Scenario IDs - Add new scenarios at the end.
 */

#define SND_SCN_PLAYBACK_SPEAKER        "playback speaker"
#define SND_SCN_PLAYBACK_HEADPHONES     "playback headphone"
#define SND_SCN_PLAYBACK_HEADSET        "playback headset"
#define SND_SCN_PLAYBACK_BLUETOOTH      "playback bluetooth"
#define SND_SCN_PLAYBACK_HANDSET        "playback handset"
#define SND_SCN_PLAYBACK_GSM            "playback gsm"
#define SND_SCN_PLAYBACK_LINE           "playback line"

#define SND_SCN_CAPTURE_MIC             "capture mic"
#define SND_SCN_CAPTURE_LINE            "capture line"
#define SND_SCN_CAPTURE_HEADSET         "capture headset"
#define SND_SCN_CAPTURE_HANDSET         "capture handset"
#define SND_SCN_CAPTURE_BLUETOOTH       "capture bluetooth"
#define SND_SCN_CAPTURE_GSM             "capture gsm"

#define SND_SCN_PHONECALL_GSM_HANDSET   "phonecall gsm handset"
#define SND_SCN_PHONECALL_BT_HANDSET    "phonecall bt handset"
#define SND_SCN_PHONECALL_IP_HANDSET    "phonecall ip handset"
#define SND_SCN_PHONECALL_GSM_HEADSET   "phonecall gsm headset"
#define SND_SCN_PHONECALL_BT_HEADSET    "phonecall bt headset"
#define SND_SCN_PHONECALL_IP_HEADSET    "phonecall ip headset"

/**
 * QOS
 *
 * Defines Audio Quality of Service. Systems supporting different types of QoS
 * often have lower power consumption on lower quality levels.
 */
#define SND_POWER_QOS_HIFI			0
#define SND_POWER_QOS_VOICE			1
#define SND_POWER_QOS_SYSTEM			2

struct snd_scenario;

/* TODO: add notification */

/**
 * snd_scenario_list - list supported scenarios
 * @scn: scenario
 * @list: list of supported scenario names.
 *
 * List supported scenarios for this sound card.
 * Returns number of scenarios.
 */
int snd_scenario_list(struct snd_scenario *scn, const char **list[]);

/**
 * snd_scenario_set_scn - set scenario
 * @scn: scenario
 * @scenario: scenario name
 *
 * Set new scenario for sound card.
 */
int snd_scenario_set_scn(struct snd_scenario *scn, const char *scenario);

/**
 * snd_scenario_get_scn - get scenario
 * @scn: scenario
 *
 * Get current sound card scenario.
 */
const char *snd_scenario_get_scn(struct snd_scenario *scn);

/**
 * snd_scenario_get_master_playback_volume - get playback volume
 * @scn: scenario
 *
 * Get the master playback volume control name for the current scenario.
 */
int snd_scenario_get_master_playback_volume(struct snd_scenario *scn);

/**
 * snd_scenario_get_master_playback_switch - get playback switch
 * @scn: scenario
 *
 * Get the master playback switch control name for the current scenario.
 */
 int snd_scenario_get_master_playback_switch(struct snd_scenario *scn);

/**
 * snd_scenario_get_master_capture_volume - get capture volume
 * @scn: scenario
 *
 * Get the master capture volume control name for the current scenario.
 */
int snd_scenario_get_master_capture_volume(struct snd_scenario *scn);

/**
 * snd_scenario_get_master_capture_switch - get capture switch
 * @scn: scenario
 *
 * Get the master capture switch control name for the current scenario.
 */
int snd_scenario_get_master_capture_switch(struct snd_scenario *scn);

/**
 * snd_scenario_set_qos - set qos
 * @qos: qos
 *
 * Set Quality of Service for this scenario.
 */
int snd_scenario_set_qos(struct snd_scenario *scn, int qos);

/**
 * snd_scenario_get_qos - get qos
 * @scn: scenario
 *
 * Get Quality of Service for this scenario.
 */
int snd_scenario_get_qos(struct snd_scenario *scn);

/**
 * snd_scenario_open - open scenario core
 * @card_name: sound card name.
 *
 * Open scenario manager core for sound card.
 */
struct snd_scenario *snd_scenario_open(const char *card_name);

/**
 * snd_scenario_reload - reload and reparse scenario configuration
 * @scn: scenario
 *
 * Reloads and reparses sound card scenario configuration.
 */
int snd_scenario_reload(struct snd_scenario *scn);

/**
 * snd_scenario_close - close scenario core
 * @scn: scenario
 *
 * Free scenario manager core for sound card.
 */
void snd_scenario_close(struct snd_scenario *scn);

/**
 * snd_scenario_dump - dump
 * @card_name: sound card name.
 *
 * Dump current sound card settings to stdout in scn format.
 */
int snd_scenario_dump(const char *card_name);
