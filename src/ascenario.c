/*
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Copyright (C) 2008-2009 SlimLogic Ltd
 *  Authors: Liam Girdwood <lrg@slimlogic.co.uk>
 *	     Stefan Schmidt <stefan@slimlogic.co.uk>
 */

#define _GNU_SOURCE /* needed of O_NOATIME */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <alsa/asoundlib.h>

#include "../include/ascenario.h"

#define PRE_SEQ		0
#define POST_SEQ	1
#define MAX_SCN		32
#define MAX_NAME	64
#define MAX_FILE	256
#define MAX_BUF		256
#define ALSA_SCN_DIR	"/etc/alsa/scenario"

/*
 * Stores all scenario settings for 1 kcontrol. Hence we have a
 * control_settings for each kcontrol in card.
 */
struct control_settings {
	char name[MAX_NAME];
	int id;
	snd_ctl_elem_type_t type;
	int count; /* 1 = mono, 2 = stereo, etc */
	unsigned short *value; /* kcontrol value 2D array */
};

/*
 * If sleep is 0 the element contains the settings in control. Else sleep
 * contains the sleep time in micro seconds.
 */
struct sequence_element {
	unsigned int sleep; /* Sleep time in msecs if sleep element, else 0 */
	struct control_settings *control;
	struct sequence_element *next; /* Pointer to next list element */
};

/*
 * Describes default mixers and qos for scenario.
 * We have a scenario_info for each scenario loaded.
 */
struct scenario_info {
	char *name;
	char *file;
	char *pre_sequence_file;
	char *post_sequence_file;
	short playback_volume_id;
	short playback_switch_id;
	short capture_volume_id;
	short capture_switch_id;
	int qos;
};

/* Describe a snd card and all its scenarios.
 */
struct snd_scenario {
	char *card_name;
	int current_scenario;
	int num_scenarios; /* number of supported scenarios */
	int num_kcontrols;  /* number of kcontrols */
	struct sequence_element *pre_seq_list; /* Linked list for pre sequence */
	struct sequence_element *post_seq_list; /* Linked list for post sequence */
	const char **list;
	struct scenario_info *scenario; /* var len array of scenario info */
	struct control_settings *control; /* var len array of controls */
};

static void scn_error(const char *fmt,...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "scenario: ");
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static void scn_stdout(const char *fmt,...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stdout, fmt, va);
	va_end(va);
}

static inline void set_value(struct snd_scenario *scn,
	struct control_settings *control, int count, unsigned short val)
{
	int offset = scn->current_scenario * control->count;
	control->value[offset + count] = val;
}

static inline unsigned short get_value(struct snd_scenario *scn,
	struct control_settings *control, int count)
{
	int offset = scn->current_scenario * control->count;
	return control->value[offset + count];
}

static int dump_control(snd_ctl_t *handle, snd_ctl_elem_id_t *id)
{
	int err, count, i;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_type_t type;
	snd_ctl_elem_value_t *control;

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_info_set_id(info, id);
	err = snd_ctl_elem_info(handle, info);
	if (err < 0) {
		scn_stdout("%s: failed to get ctl info\n");
		return err;
	}

	snd_ctl_elem_value_set_id(control, id);
	snd_ctl_elem_read(handle, control);

	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	if (count == 0)
		return 0;

	scn_stdout("%u:'%s':%d:",
	       snd_ctl_elem_id_get_numid(id),
	       snd_ctl_elem_id_get_name(id), count);

	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		for (i = 0; i < count - 1; i++)
			scn_stdout("%d,",
				snd_ctl_elem_value_get_boolean(control, i));
		scn_stdout("%d", snd_ctl_elem_value_get_boolean(control, i));
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		for (i = 0; i < count - 1; i++)
			scn_stdout("%d,",
				snd_ctl_elem_value_get_integer(control, i));
		scn_stdout("%d", snd_ctl_elem_value_get_integer(control, i));
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		for (i = 0; i < count - 1; i++)
			scn_stdout("%ld,",
				snd_ctl_elem_value_get_integer64(control, i));
		scn_stdout("%ld",
				snd_ctl_elem_value_get_integer64(control, i));
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		for (i = 0; i < count - 1; i++)
			scn_stdout("%d,",
				snd_ctl_elem_value_get_enumerated(control, i));
		scn_stdout("%d",
				snd_ctl_elem_value_get_enumerated(control, i));
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
		for (i = 0; i < count - 1; i++)
			scn_stdout("%2.2x,",
				snd_ctl_elem_value_get_byte(control, i));
		scn_stdout("%2.2x", snd_ctl_elem_value_get_byte(control, i));
		break;
	default:
		break;
	}
	scn_stdout("\n");
	return 0;
}

/*
 * Add new kcontrol from sound card into memory database.
 */
static int add_control(snd_ctl_t *handle, snd_ctl_elem_id_t *id,
	struct control_settings *control_settings)
{
	int err;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_info_set_id(info, id);
	err = snd_ctl_elem_info(handle, info);
	if (err < 0) {
		scn_stdout("%s: failed to get ctl info\n");
		return err;
	}

	snd_ctl_elem_value_set_id(control, id);
	snd_ctl_elem_read(handle, control);

	strncpy(control_settings->name, snd_ctl_elem_id_get_name(id),
		MAX_NAME);
	control_settings->count = snd_ctl_elem_info_get_count(info);
	control_settings->type = snd_ctl_elem_info_get_type(info);
	control_settings->id = snd_ctl_elem_id_get_numid(id);
	return 0;
}

static int parse_controls(struct snd_scenario *scn, FILE *f)
{
	struct control_settings *control;
	char buf[MAX_BUF], name[MAX_NAME];
	int id, count, line = 1, i;
	char *name_start, *name_end, *tbuf;

	while (fgets(buf, MAX_BUF, f) != NULL) {

		/* get id */
		tbuf = buf;
		id = atoi(tbuf);
		if (id == 0) {
			scn_error("%s:id == 0 on line %d\n", __func__, line);
			return -EINVAL;
		}
		for (i = 0; i < scn->num_kcontrols; i++) {
			if (id == scn->control[i].id) {
				control = &scn->control[i];
				goto get_name;
			}
		}
		scn_error("%s:id not found at line %d\n", __func__, line);
			return -EINVAL;
get_name:
		/* get name start */
		while (*tbuf != 0 && *tbuf != '\'')
			tbuf++;
		if (*tbuf == 0)
			return -EINVAL;
		name_start = ++tbuf;

		/* get name end */
		while (*tbuf != 0 && *tbuf != '\'')
			tbuf++;
		if (*tbuf == 0)
			return -EINVAL;
		name_end = tbuf++;

		/* copy name */
		if ((name_end - name_start) > MAX_NAME) {
			scn_error("%s:name too big at %d chars line %d\n",
				 __func__, name_end - name_start, line);
			return -EINVAL;
		}
		strncpy(name, name_start, name_end - name_start);
		name[name_end - name_start] = 0;
		if (strcmp(name, control->name)) {
			scn_error("%s: name %s and %s don't match at line %d\n",
				 __func__, name, control->name, line);
			return -EINVAL;
		}

		/* get count */
		tbuf++;
		count = atoi(tbuf);
		if (count == 0) {
			scn_error("%s:count == 0 on line %d\n", __func__,
				line);
			return -EINVAL;
		}
		if (count != control->count) {
			scn_error("%s:count does not match at line %d\n",
				__func__, line);
			return -EINVAL;
		}

		/* get vals */
		control->value = malloc(control->count * scn->num_scenarios *
			sizeof(unsigned short));
		if (control->value == NULL)
			return -ENOMEM;

		while (*tbuf != 0 && *tbuf != ':')
			tbuf++;
		if (*tbuf == 0)
			return -EINVAL;
		tbuf++;

		for (i = 0; i < count; i++) {
			set_value(scn, control, i, atoi(tbuf));
			while (*tbuf != 0 && *tbuf != ',')
				tbuf++;

			if (*tbuf++ == 0 && i < (count - 1))
				return -EINVAL;
		}
		line++;
	}

	return 0;
}

static char *get_string (char *buf)
{
	char *str, *end;

	/* find '=' */
	while (isblank(*buf))
		buf++;
	if (*buf == 0 || *buf != '=') {
		scn_error("%s: missing '='\n", __func__);
		return NULL;
	}

	/* find leading '"' */
	buf++;
	while (isblank(*buf))
		buf++;
	if (*buf == 0 || *buf != '"') {
		scn_error("%s: missing start '\"'\n", __func__);
		return NULL;
	}
	str = ++buf;

	/* get value */
	while (*buf != 0 && *buf != '"')
		buf++;
	end = buf;

	/* find '"' terminator */
	if (*buf == 0 || *buf != '"') {
		scn_error("%s: missing terminator '\"' %s\n", __func__, buf);
		return NULL;
	}

	*end = 0;
	return strdup(str);
}

static char *get_control_name (char *buf)
{
	char *str, *end;

	/* find leading '"' */
	buf++;
	while (isblank(*buf))
		buf++;
	if (*buf == 0 || *buf != '"') {
		scn_error("%s: missing start '\"'\n", __func__);
		return NULL;
	}
	str = ++buf;

	/* get value */
	while (*buf != 0 && *buf != '"')
		buf++;
	end = buf;

	/* find '"' terminator */
	if (*buf == 0 || *buf != '"') {
		scn_error("%s: missing terminator '\"' %s\n", __func__, buf);
		return NULL;
	}

	*end = 0;
	return strdup(str);
}

static int get_int (char *buf)
{
	/* find '=' */
	while (isblank(*buf))
		buf++;
	if (*buf == 0 || *buf != '=') {
		scn_error("%s: missing '='\n", __func__);
		return -EINVAL;
	}
	buf++;
	return atoi(buf);
}

static int get_enum (char *buf)
{
	/* find '=' */
	while (isblank(*buf))
		buf++;
	if (*buf == 0 || *buf != '=') {
		scn_error("%s: missing '='\n", __func__);
		return -EINVAL;
	}
	buf++;
	return 0; /* TODO */
}

static void seq_list_append(struct snd_scenario *scn,
			struct sequence_element *curr, int position)
{
	struct sequence_element *last, *tmp;

	if (position) {
		if (!scn->post_seq_list)
			scn->post_seq_list = curr;

		else {
			tmp = scn->post_seq_list;
			while (tmp) {
				last = tmp;
				tmp = tmp->next;
			}
			last->next = curr;
		}
	}
	else {
		if (!scn->pre_seq_list) {
			scn->pre_seq_list = curr;
		}
		else {
			tmp = scn->pre_seq_list;
			while (tmp) {
				last = tmp;
				tmp = tmp->next;
			}
			last->next = curr;
		}
	}
}

static int parse_sequences(struct snd_scenario *scn, FILE *f, int position)
{
	char buf[MAX_BUF], *tbuf, *control_value;
	int control_len, i;
	struct sequence_element *curr;

	while (fgets(buf, MAX_BUF, f) != NULL) {

		/* Check for lines with comments and ignore */
		if (buf[0] == '#')
			continue;

		/* Parse current line and skip blanks */
		tbuf = buf;
		while (isblank(*tbuf))
			tbuf++;

		curr = malloc(sizeof(struct sequence_element));
		if (curr == NULL)
			return -ENOMEM;
		bzero(curr, sizeof(struct sequence_element));

		curr->control = malloc(sizeof(struct control_settings));
		if (curr->control == NULL)
			return -ENOMEM;
		bzero(curr->control, sizeof(struct control_settings));

		curr->control->value = malloc(curr->control->count * scn->num_scenarios
						* sizeof(unsigned short));
		if (curr->control->value == NULL)
			return -ENOMEM;
		bzero(curr->control->value, curr->control->count * scn->num_scenarios
				* sizeof(unsigned short));

		if (strncmp(tbuf, "kcontrol", 8) == 0) {
			strncpy(curr->control->name, get_control_name(tbuf + 8), MAX_NAME);
			control_len = strlen(curr->control->name);
			/* 11 = 8 from kcontrol + 2 quotes + 1 blank */
			control_value = get_string(tbuf + 11 + control_len);

			for (i = 0; i < scn->num_kcontrols; i++) {
				if (strncmp(curr->control->name, scn->control[i].name,
						control_len) == 0) {
					curr->sleep = 0;
					curr->control->id = scn->control[i].id;
					curr->control->type = scn->control[i].type;
					curr->control->count = scn->control[i].count;
					set_value(scn, curr->control, curr->control->count,
							atoi(control_value));
					seq_list_append(scn, curr, position);
				}
			}

			continue;
		}

		if (strncmp(tbuf, "msleep", 6) == 0) {
			curr->sleep = get_int(tbuf + 6);

			/* Free control elements as we only have a sleep element
			 * here */
			if (curr->control) {
				if (curr->control->value)
					free(curr->control->value);
				free(curr->control);
			}

			seq_list_append(scn, curr, position);
			continue;
		}
	}

	return 0;
}

/* load scenario i */
static int read_scenario_file(struct snd_scenario *scn)
{
	int fd, ret;
	FILE *f;
	char filename[MAX_FILE];
	struct scenario_info *info = &scn->scenario[scn->current_scenario];

	snprintf(filename, MAX_FILE, "%s/%s/%s", ALSA_SCN_DIR, scn->card_name,
		info->file);

	fd = open(filename, O_RDONLY | O_NOATIME);
	if (fd < 0) {
		scn_error("%s: couldn't open %s\n", __func__, filename);
		return fd;
	}

	f = fdopen(fd, "r");
	if (f == NULL) {
		ret = errno;
		goto close;
	}

	ret = parse_controls(scn, f);
	fclose(f);
close:
	close(fd);
	return ret;
}

static int read_sequence_file(struct snd_scenario *scn, int position)
{
	int fd, ret;
	FILE *f;
	char filename[MAX_FILE];
	struct scenario_info *info = &scn->scenario[scn->current_scenario];

	if (position == PRE_SEQ) {
		sprintf(filename, "%s/%s/%s", ALSA_SCN_DIR, scn->card_name,
			info->pre_sequence_file);
	}
	else {
		sprintf(filename, "%s/%s/%s", ALSA_SCN_DIR, scn->card_name,
			info->post_sequence_file);
	}

	fd = open(filename, O_RDONLY | O_NOATIME);
	if (fd < 0) {
		return fd;
	}

	f = fdopen(fd, "r");
	if (f == NULL) {
		ret = errno;
		goto close;
	}

	ret = parse_sequences(scn, f, position);
	fclose(f);
close:
	close(fd);
	return ret;
}

static int parse_scenario(struct snd_scenario *scn, FILE *f, int line_)
{
	struct scenario_info *info;
	int line = line_ - 1, id = 0, file = 0;
	char buf[MAX_BUF], *tbuf;

	scn->scenario = realloc(scn->scenario,
		(scn->num_scenarios + 1) * sizeof(struct scenario_info));
	if (scn->scenario == NULL)
		return -ENOMEM;
	bzero(scn->scenario, sizeof(struct scenario_info));
	info = scn->scenario + scn->num_scenarios;

	/* Set sequence filename to NULL as it is optional and we want to check
	 * for  NULL to avoid segfaults */
	info->pre_sequence_file = NULL;
	info->post_sequence_file = NULL;

	while(fgets(buf, MAX_BUF, f) != NULL) {

		line++;
		if (buf[0] == '#')
			continue;

		tbuf = buf;
		while (isblank(*tbuf))
			tbuf++;

		if (strncmp(tbuf, "Identifier", 10) == 0) {
			info->name = get_string(tbuf + 10);
			if (info->name == NULL) {
				scn_error("%s: failed to get Identifer\n",
					__func__);
				goto err;
			}
			id = 1;
			continue;
		}

		if (strncmp(tbuf, "File", 4) == 0) {
			info->file = get_string(tbuf + 4);
			if (info->file == NULL) {
				scn_error("%s: failed to get File\n",
					__func__);
				goto err;
			}
			file = 1;
			continue;
		}

		if (strncmp(tbuf, "QoS", 3) == 0) {
			info->qos = get_enum(tbuf + 3);
			if (info->qos < 0) {
				scn_error("%s: failed to get QoS\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "MasterPlaybackVolume", 20) == 0) {
			info->playback_volume_id = get_int(tbuf + 20);
			if (info->playback_volume_id < 0) {
				scn_error("%s: failed to get MasterPlaybackVolume\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "MasterPlaybackSwitch", 20) == 0) {
			info->playback_switch_id = get_int(tbuf + 20);
			if (info->playback_switch_id < 0) {
				scn_error("%s: failed to get MasterPlaybackSwitch\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "MasterCaptureVolume", 19) == 0) {
			info->capture_volume_id = get_int(tbuf + 19);
			if (info->capture_volume_id < 0) {
				scn_error("%s: failed to get MasterCaptureVolume\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "MasterCaptureSwitch", 19) == 0) {
			info->capture_switch_id = get_int(tbuf + 19);
			if (info->capture_switch_id < 0) {
				scn_error("%s: failed to get MasterCaptureSwitch\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "PreSequenceFile", 15) == 0) {
			info->pre_sequence_file = get_string(tbuf + 15);
			if (info->pre_sequence_file == NULL) {
				scn_error("%s: failed to get PreSequenceFile\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "PostSequenceFile", 16) == 0) {
			info->post_sequence_file = get_string(tbuf + 16);
			if (info->post_sequence_file == NULL) {
				scn_error("%s: failed to get PostSequenceFile\n",
					__func__);
				goto err;
			}
			continue;
		}

		if (strncmp(tbuf, "EndSection", 10) == 0) {
			break;
		}
	}

	if (file & id) {
		scn->num_scenarios++;
		return 0;
	}
err:
	if (file) {
		free(info->file);
		info->file = NULL;
	}
	if (id) {
		free(info->name);
		info->name = NULL;
	}
	return -EINVAL;
}

static int read_master_file(struct snd_scenario *scn, FILE *f)
{
	int line = 0, ret = 0, i;
	char buf[MAX_BUF], *tbuf;

	/* parse master config sections */
	while(fgets(buf, MAX_BUF, f) != NULL) {

		if (buf[0] == '#') {
			line++;
			continue;
		}

		if (strncmp(buf, "Section", 7) == 0) {

			tbuf = buf + 7;
			while (isblank(*tbuf))
				tbuf++;

			if (strncmp(tbuf, "\"Scenario\"", 10) == 0) {
				line = parse_scenario(scn, f, line);
				if (line < 0) {
					scn_error("%s: failed to parse "
						"scenario\n", __func__);
					goto err;
				}
				continue;
			}
		}
		line++;
	}

	/* copy ptrs to scenario names */
	scn->list = malloc(scn->num_scenarios * sizeof(char *));
	if (scn->list == NULL)
		ret = -ENOMEM;
	for (i = 0; i < scn->num_scenarios; i++)
		scn->list[i] = scn->scenario[i].name;

err:
	if (ferror(f)) {
		scn_error("%s: failed to read master\n", __func__);
		return ferror(f);
	}
	return ret;
}

/* load scenario i */
static int import_master_config(struct snd_scenario *scn)
{
	int fd, ret;
	FILE *f;
	char filename[MAX_FILE];

	sprintf(filename, "%s/%s.conf", ALSA_SCN_DIR, scn->card_name);

	fd = open(filename, O_RDONLY | O_NOATIME);
	if (fd < 0) {
		scn_error("%s: couldn't open %s\n", __func__, filename);
		return fd;
	}

	f = fdopen(fd, "r");
	if (f == NULL) {
		ret = errno;
		goto close;
	}

	ret = read_master_file(scn, f);
	fclose(f);
close:
	close(fd);
	return ret;
}

/* parse_card_controls
 * @scn: scenario
 *
 * Parse sound card and store control data in memory db.
 */
static int parse_card_controls(struct snd_scenario *scn)
{
	struct control_settings *control;
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_ctl_elem_list_t *list;
	int ret, i;

	snd_ctl_card_info_alloca(&info);
	snd_ctl_elem_list_alloca(&list);

	/* open and load snd card */
	ret = snd_ctl_open(&handle, scn->card_name, SND_CTL_READONLY);
	if (ret < 0) {
		scn_error("%s: control %s open retor: %s\n", __func__,
			scn->card_name, snd_strerror(ret));
		return ret;
	}

	ret = snd_ctl_card_info(handle, info);
	if (ret < 0) {
		scn_error("%s :control %s local retor: %s\n", __func__,
			scn->card_name, snd_strerror(ret));
		goto close;
	}

	ret = snd_ctl_elem_list(handle, list);
	if (ret < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto close;
	}

	scn->num_kcontrols = snd_ctl_elem_list_get_count(list);
	if (scn->num_kcontrols < 0) {
		ret = 0;
		goto close;
	}

	snd_ctl_elem_list_set_offset(list, 0);
	if (snd_ctl_elem_list_alloc_space(list, scn->num_kcontrols) < 0) {
		scn_error("%s: not enough memory...\n", __func__);
		ret =  -ENOMEM;
		goto close;
	}
	if ((ret = snd_ctl_elem_list(handle, list)) < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto free;
	}

	/* allocate db memory for controls */
	scn->control = calloc(scn->num_kcontrols,
		sizeof(struct control_settings));
	if (scn->control == NULL) {
		ret = -ENOMEM;
		goto free;
	}
	control = scn->control;

	/* iterate through each kcontrol and add to db */
	for (i = 0; i < scn->num_kcontrols; ++i) {
		snd_ctl_elem_id_t *id;
		snd_ctl_elem_id_alloca(&id);
		snd_ctl_elem_list_get_id(list, i, id);

		ret = add_control(handle, id, control++);
		if (ret < 0) {
			scn_error("%s: failed to add control error %s\n",
				__func__, snd_strerror(ret));
			goto close;
		}
	}
free:
	snd_ctl_elem_list_free_space(list);
close:
	snd_ctl_close(handle);
	return ret;
}

/* import_scenario_files -
 * @scn: scenario
 *
 * Read and parse scenario_info files the store in memory.
 */
static int import_scenario_files(struct snd_scenario *scn)
{
	int ret;

	ret = import_master_config(scn);
	if (ret < 0) {
		scn_error("%s: failed to parse master scenario config\n",
			__func__);
		return ret;
	}

	for (scn->current_scenario = 0;
		scn->current_scenario < scn->num_scenarios;
		scn->current_scenario++) {

		ret = read_scenario_file(scn);
		if (ret < 0) {
			scn_error("%s: failed to parse scenario %s\n",
				__func__,
				scn->scenario[scn->current_scenario].name);
				scn->current_scenario = -1;
			return ret;
		}

		if (scn->scenario[scn->current_scenario].pre_sequence_file != NULL) {
			ret = read_sequence_file(scn, PRE_SEQ);
			if (ret < 0) {
				scn_stdout("Warning: PreSequence file defined but"
					" missing in scenario \"%s\"\n",
					scn->scenario[scn->current_scenario].name);
			}
		}

		if (scn->scenario[scn->current_scenario].post_sequence_file != NULL) {
			ret = read_sequence_file(scn, POST_SEQ);
			if (ret < 0) {
				scn_stdout("Warning: PostSequence file defined but"
					" missing in scenario \"%s\"\n",
					scn->scenario[scn->current_scenario].name);
			}
		}

	}
	return 0;
}

/* free all resorces */
static void free_scn(struct snd_scenario *scn)
{
	/* TODO: valgrind to make sure. */
	int i;

	if (scn == NULL)
		return;

	if (scn->control) {
		if (scn->control->value)
			free(scn->control->value);
		free(scn->control);
	}

	if (scn->list)
		free(scn->list);
	if (scn->card_name)
		free(scn->card_name);
	if (scn->pre_seq_list)
		free(scn->pre_seq_list);
	if (scn->post_seq_list)
		free(scn->post_seq_list);

	if (scn->scenario) {
		for (i = 0; i < scn->num_scenarios; i++) {
			struct scenario_info *info = &scn->scenario[i];

			if (info->name)
				free(info->name);
			if (info->file)
				free(info->file);
			if (info->pre_sequence_file)
				free(info->pre_sequence_file);
			if (info->post_sequence_file)
				free(info->post_sequence_file);
		}
		free(scn->scenario);
	}
	free(scn);
}

/*
 * Init sound card scenario db.
 */
struct snd_scenario *snd_scenario_open(const char *card_name)
{
	struct snd_scenario *scn;
	int err;

	/* TODO: locking and
	 * check if card_name scn is already loaded,
	 * if so reuse to conserve ram. */

	scn = malloc(sizeof(struct snd_scenario));
	if (scn == NULL)
		return NULL;
	bzero(scn, sizeof(struct snd_scenario));
	scn->card_name = strdup(card_name);
	if (scn->card_name == NULL) {
		free(scn);
		return NULL;
	}

	/* get info about sound card */
	err = parse_card_controls(scn);
	if (err < 0) {
		free_scn(scn);
		return NULL;
	}

	/* get info on scenarios and verify against card */
	err = import_scenario_files(scn);
	if (err < 0) {
		free_scn(scn);
		return NULL;
	}

	return scn;
}

/*
 * Reload and reparse scenario db.
 */
int snd_scenario_reload(struct snd_scenario *scn)
{
	free_scn(scn);

	scn->num_kcontrols = parse_card_controls(scn);
	if (scn->num_kcontrols <= 0) {
		free_scn(scn);
		return -EINVAL;
	}

	scn->num_scenarios = import_scenario_files(scn);
	if (scn->num_scenarios <= 0) {
		return -EINVAL;
	}

	return 0;
}

void snd_scenario_close(struct snd_scenario *scn)
{
	free_scn(scn);
}

static int set_control(snd_ctl_t *handle, snd_ctl_elem_id_t *id,
	struct snd_scenario *scn)
{
	struct control_settings *setting;
	int ret, count, i, idnum;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_type_t type;
	snd_ctl_elem_value_t *control;

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_info_set_id(info, id);
	ret = snd_ctl_elem_info(handle, info);
	if (ret < 0) {
		scn_error("%s: failed to get ctl info\n", __func__);
		return ret;
	}

	snd_ctl_elem_value_set_id(control, id);
	snd_ctl_elem_read(handle, control);

	idnum = snd_ctl_elem_id_get_numid(id);
	for (i = 0; i < scn->num_kcontrols; i++) {
		setting = &scn->control[i];
		if (setting->id == idnum)
			goto set_val;
	}
	scn_error("%s: failed to find control %d\n", __func__, idnum);
	return 0;

set_val:
	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	if (count == 0)
		return 0;

	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		for (i = 0; i < count; i++)
			snd_ctl_elem_value_set_boolean(control, i,
				get_value(scn, setting, i));
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		for (i = 0; i < count; i++)
			snd_ctl_elem_value_set_integer(control, i,
				get_value(scn, setting, i));

		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		for (i = 0; i < count; i++)
			snd_ctl_elem_value_set_integer64(control, i,
				get_value(scn, setting, i));

		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		for (i = 0; i < count; i++)
			snd_ctl_elem_value_set_enumerated(control, i,
				get_value(scn, setting, i));

		break;
	case SND_CTL_ELEM_TYPE_BYTES:
		for (i = 0; i < count; i++)
			snd_ctl_elem_value_set_byte(control, i,
				get_value(scn, setting, i));
		break;
	default:
		break;
	}

	ret = snd_ctl_elem_write(handle, control);
	if (ret < 0) {
		scn_error("%s: control %s failed: %s\n", __func__,
			 setting->name, snd_strerror(ret));
		scn_error("%s: count %d type: %d\n", __func__,
			count, type);
		for (i = 0; i < count; i++)
			fprintf(stderr, "%d ", get_value(scn, setting, i));
		return ret;
	}
	return 0;
}

static void exec_sequence(struct sequence_element *seq, struct snd_scenario
			*scn, snd_ctl_elem_list_t *list, snd_ctl_t *handle)
{
	int count = snd_ctl_elem_list_get_count(list);
	while (seq) {
		if (seq->sleep)
			usleep(seq->sleep);
		else {
			snd_ctl_elem_id_t *id;
			snd_ctl_elem_id_alloca(&id);
			int ret, i, numid;
			/* Where is id lookup from numid if you need it? */
			for (i = 0; i < count; ++i) {
				snd_ctl_elem_list_get_id(list, i, id);
				numid = snd_ctl_elem_id_get_numid(id);
				if (numid == seq->control->id) {
					ret = set_control(handle, id, scn);
					if (ret < 0) {
						scn_error("%s: failed to set control %s\n",
							__func__, scn->card_name);
					}
					break;
				}
			}
		}
		seq = seq->next;
	}
}

int snd_scenario_set_scn(struct snd_scenario *scn, const char *name)
{
	snd_ctl_card_info_t *info;
	snd_ctl_elem_list_t *list;
	snd_ctl_t *handle;
	int ret, count, i;

	snd_ctl_card_info_alloca(&info);
	snd_ctl_elem_list_alloca(&list);

	/* find scenario name */
	for (i = 0; i < scn->num_scenarios; i++) {
		if (!strcmp(scn->scenario[i].name, name))
			goto found;
	}
	scn_error("%s: scenario %s not found\n", __func__, name);
	return -EINVAL;

found:
	/* scenario found - now open card */
	scn->current_scenario = i;
	ret = snd_ctl_open(&handle, scn->card_name, 0);
	if (ret) {
		scn_error("%s: control %s open error: %s\n", __func__,
			scn->card_name, snd_strerror(ret));
		return ret;
	}

	ret = snd_ctl_card_info(handle, info);
	if (ret < 0) {
		scn_error("%s :control %s local retor: %s\n", __func__,
			scn->card_name, snd_strerror(ret));
		goto close;
	}

	ret = snd_ctl_elem_list(handle, list);
	if (ret < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto close;
	}

	count = snd_ctl_elem_list_get_count(list);
	if (count < 0) {
		ret = 0;
		goto close;
	}

	snd_ctl_elem_list_set_offset(list, 0);
	if (snd_ctl_elem_list_alloc_space(list, count) < 0) {
		scn_error("%s: not enough memory...\n", __func__);
		ret =  -ENOMEM;
		goto close;
	}
	if ((ret = snd_ctl_elem_list(handle, list)) < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto free;
	}

	/* If we have a sequence list that should be executed before the new
	 * scenario is set do it now */
	if (scn->pre_seq_list)
		exec_sequence(scn->pre_seq_list, scn, list, handle);

	/* iterate through each kcontrol and add to db */
	for (i = 0; i < count; ++i) {
		snd_ctl_elem_id_t *id;
		snd_ctl_elem_id_alloca(&id);
		snd_ctl_elem_list_get_id(list, i, id);

		ret = set_control(handle, id, scn);
		if (ret < 0) {
			scn_error("%s: failed to set control %s\n", __func__,
				scn->card_name);
		}
	}

	/* If we have a sequence list that should be executed after the new
	 * scenario is set do it now */
	if (scn->post_seq_list)
		exec_sequence(scn->post_seq_list, scn, list, handle);

free:
	snd_ctl_elem_list_free_space(list);
close:
	snd_ctl_close(handle);
	return ret;
}

int snd_scenario_dump(const char *card_name)
{
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_ctl_elem_list_t *list;
	int ret, i, count;

	snd_ctl_card_info_alloca(&info);
	snd_ctl_elem_list_alloca(&list);

	/* open and load snd card */
	ret = snd_ctl_open(&handle, card_name, SND_CTL_READONLY);
	if (ret < 0) {
		scn_error("%s: control %s open retor: %s\n", __func__, card_name,
			snd_strerror(ret));
		return ret;
	}

	ret = snd_ctl_card_info(handle, info);
	if (ret < 0) {
		scn_error("%s :control %s local retor: %s\n", __func__,
			card_name, snd_strerror(ret));
		goto close;
	}

	ret = snd_ctl_elem_list(handle, list);
	if (ret < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto close;
	}

	count = snd_ctl_elem_list_get_count(list);
	if (count < 0) {
		ret = 0;
		goto close;
	}

	snd_ctl_elem_list_set_offset(list, 0);
	if (snd_ctl_elem_list_alloc_space(list, count) < 0) {
		scn_error("%s: not enough memory...\n", __func__);
		ret =  -ENOMEM;
		goto close;
	}
	if ((ret = snd_ctl_elem_list(handle, list)) < 0) {
		scn_error("%s: cannot determine controls: %s\n", __func__,
			snd_strerror(ret));
		goto free;
	}

	/* iterate through each kcontrol and add to db */
	for (i = 0; i < count; ++i) {
		snd_ctl_elem_id_t *id;
		snd_ctl_elem_id_alloca(&id);
		snd_ctl_elem_list_get_id(list, i, id);

		ret = dump_control(handle, id);
		if (ret < 0) {
			scn_error("%s: cannot determine controls: %s\n",
				__func__, snd_strerror(ret));
			goto free;
		}
	}
free:
	snd_ctl_elem_list_free_space(list);
close:
	snd_ctl_close(handle);
	return ret;
}

const char *snd_scenario_get_scn(struct snd_scenario *scn)
{
	if (scn->current_scenario > 0 && scn->current_scenario < MAX_SCN)
		return scn->scenario[scn->current_scenario].name;
	else
		return NULL;
}

int snd_scenario_set_integer(struct snd_scenario *scn, int type, int value)
{
	switch (type) {
	case SND_SCN_INT_QOS:
		scn->scenario[scn->current_scenario].qos = qos;
		return 0;
	default:
		return -EINVAL;
	}
}

int snd_scenario_get_integer(struct snd_scenario *scn, int type, int *value)
{
	if (value == NULL)
		return -EINVAL;
	switch (type) {
	case SND_SCN_INT_QOS:
		*value = scn->scenario[scn->current_scenario].qos;
		return 0;
	default:
		return -EINVAL;
	}
}

int snd_scenario_get_control_id(struct snd_scenario *scn, int type,
				snd_ctl_elem_id_t *id)
{
	/* not yet implemented */
	return -EINVAL;
}

int snd_scenario_list(struct snd_scenario *scn, const char **list[])
{
	if (scn == NULL || list == NULL)
		return -EINVAL;
	*list = scn->list;
	return scn->num_scenarios;
}
