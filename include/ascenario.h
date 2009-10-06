/**
 * \file include/ascenario.h
 * \brief Scenario interface for the ALSA driver
 * \author Liam Girdwood <lrg@slimlogic.co.uk>
 * \author Stefan Schmidt <stefan@slimlogic.co.uk>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2008-2009
 */
/*
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
 */

#ifndef __ALSA_ASCENARIO_H
#define __ALSA_ASCENARIO_H

/*! \page ascenario Scenario interface

It allows switching audio settings between scenarios or uses-cases like
listening to music and answering an incoming phone call. Made of control
aliasing for playback, capture master and switch as well as the option to
post- and prefix a sequence of control changes avoiding pops and other
unwanted noise.

*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup AScenario Scenario Interface
 *  The ALSA scenario interface.
 *  See \ref ascenario page for more details.
 *  \{
 */
     
/**
 * Scenario IDs
 *
 * Standard Scenario IDs - Add new scenarios at the end.
 */

/** use main speaker for playback */
#define SND_SCN_PLAYBACK_SPEAKER        "playback speaker"
/** use headphone output for playback */
#define SND_SCN_PLAYBACK_HEADPHONES     "playback headphone"
/** use headset for playback */
#define SND_SCN_PLAYBACK_HEADSET        "playback headset"
/** use bluetooth interface for playback */
#define SND_SCN_PLAYBACK_BLUETOOTH      "playback bluetooth"
/** use handset interface for playback */
#define SND_SCN_PLAYBACK_HANDSET        "playback handset"
/** use gsm interface for playback */
#define SND_SCN_PLAYBACK_GSM            "playback gsm"
/** use line interface for playback */
#define SND_SCN_PLAYBACK_LINE           "playback line"

/** use mic input for capture */
#define SND_SCN_CAPTURE_MIC             "capture mic"
/** use line input for capture */
#define SND_SCN_CAPTURE_LINE            "capture line"
/** use headset input for capture */
#define SND_SCN_CAPTURE_HEADSET         "capture headset"
/** use handset input for capture */
#define SND_SCN_CAPTURE_HANDSET         "capture handset"
/** use bluetooth input for capture */
#define SND_SCN_CAPTURE_BLUETOOTH       "capture bluetooth"
/** use gsm input for capture */
#define SND_SCN_CAPTURE_GSM             "capture gsm"

/** phone call through gsm handset */
#define SND_SCN_PHONECALL_GSM_HANDSET   "phonecall gsm handset"
/** phone call through bluetooth handset */
#define SND_SCN_PHONECALL_BT_HANDSET    "phonecall bt handset"
/** phone call through ip handset */
#define SND_SCN_PHONECALL_IP_HANDSET    "phonecall ip handset"
/** phone call through gsm headset */
#define SND_SCN_PHONECALL_GSM_HEADSET   "phonecall gsm headset"
/** phone call through bluetooth headset */
#define SND_SCN_PHONECALL_BT_HEADSET    "phonecall bt headset"
/** phone call through ip headset */
#define SND_SCN_PHONECALL_IP_HEADSET    "phonecall ip headset"

/**
 * QOS
 *
 * Defines Audio Quality of Service. Systems supporting different types of QoS
 * often have lower power consumption on lower quality levels.
 */
/** use HIFI grade QoS service */
#define SND_POWER_QOS_HIFI			0
/** use voice grade QoS service */
#define SND_POWER_QOS_VOICE			1
/** use system sound grade QoS service */
#define SND_POWER_QOS_SYSTEM			2

/**
 * KControl types
 */
/** master playback volume */
#define SND_SCN_KCTL_MASTER_PLAYBACK_VOLUME	1
/** master playback switch */
#define SND_SCN_KCTL_MASTER_PLAYBACK_SWITCH	2
/** master capture volume */
#define SND_SCN_KCTL_MASTER_CAPTURE_VOLUME	3
/** master capture switch */
#define SND_SCN_KCTL_MASTER_CAPTURE_SWITCH	4

/**
 * Integer types
 */
/** QoS volume */
#define SND_SCN_INT_QOS				1

/** Scenario container */
typedef snd_scenario_t snd_scenario_t;

/* TODO: add notification */

/**
 * \brief list supported scenarios for given soundcard
 * \param scn scenario
 * \param list returned list of supported scenario names
 * \return number of scenarios if success, otherwise a negative error code
 */
int snd_scenario_list(snd_scenario_t *scn, const char **list[]);

/**
 * \brief set new scenario for sound card
 * \param scn scenario
 * \param scenario scenario id string
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_set_scn(snd_scenario_t *scn, const char *scenario);

/**
 * \brief get scenario
 * \param scn scenario
 * \return scenario id string
 *
 * Get current sound card scenario.
 */
const char *snd_scenario_get_scn(snd_scenario_t *scn);

/**
 * \brief get associated control id
 * \param scn scenario
 * \param kctl_type see SND_SCN_KCTL_* constants
 * \param id returned control id
 * \return zero if success, otherwise a negative error code
 *
 * Get the control id for the current scenario.
 */
int snd_scenario_get_kcontrol(snd_scenario_t *scn,
                              int kctl_type, snd_ctl_elem_id_t *id);

/**
 * \brief set integer value
 * \param scn scenario
 * \param int_key see SND_SCN_INT_* constants
 * \param value value
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_set_integer(snd_scenario_t *scn, int int_key, int value);

/**
 * \brief get integer value
 * \param scn scenario
 * \param int_key see SND_SCN_INT_* constants
 * \param value value
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_get_integer(snd_scenario_t *scn, int int_key, int *value);

/**
 * \brief open scenario core for sound card
 * \param card_name sound card name.
 * \return zero if success, otherwise a negative error code
 */
snd_scenario_t *snd_scenario_open(const char *card_name);

/**
 * \brief reload and reparse scenario configuration
 * \param scn scenario
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_reload(snd_scenario_t *scn);

/**
 * \brief close scenario
 * \param scn scenario
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_close(snd_scenario_t *scn);

/**
 * \brief dump scenario
 * \param output handle
 * \param card_name sound card name
 * \return zero if success, otherwise a negative error code
 */
int snd_scenario_dump(snd_output_t *output, const char *card_name);

/**
 *  \}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_ASCENARIO_H */  
