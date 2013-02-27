/****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 * modified by gave92
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Settings.cpp
 *
 * Settings Class
 * for WiiBrowser 2012
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ogcsys.h>
#include <common.h>
#include <unistd.h>

#include "Settings.h"

#define DEFAULT_APP_PATH    "apps/wiibrowser/"
#define DEFAULT_HOMEPAGE    "www.google.com/"
#define CONFIGPATH          "apps/wiibrowser/"
#define CONFIGNAME          "wiibrowser.cfg"

#define DEBUG

SSettings::SSettings()
{
    strcpy(BootDevice, "sd:/");
    snprintf(ConfigPath, sizeof(ConfigPath), "%s%s%s", BootDevice, CONFIGPATH, CONFIGNAME);
    this->SetDefault();
}

SSettings::~SSettings()
{
}

void SSettings::SetDefault()
{
    Language = LANG_ENGLISH;
    UserAgent = LIBCURL;
    Revision = 0;

    ShowTooltip = true;
    Music = true;
    Autoupdate = true;

    sprintf(Homepage, DEFAULT_HOMEPAGE);
    sprintf(AppPath, "%s%s", BootDevice, DEFAULT_APP_PATH);

    sprintf(DefaultFolder, DEFAULT_APP_PATH);
    sprintf(UserFolder, AppPath);

    for(int i = 0; i < N; i++)
    {
        Favorites[i] = new char[256];
        memset(Favorites[i], 0, 256);
    }
}

bool SSettings::Save()
{
    if(!FindConfig())
        return false;

    char filedest[100];
    snprintf(filedest, sizeof(filedest), "%s", ConfigPath);

    #ifdef DEBUG
    save_mem("Saving..");
    save_mem(filedest);
    #endif

    file = fopen(ConfigPath, "w");
    if(!file)
    {
        fclose(file);
        return false;
    }

    fprintf(file, "# WiiBrowser Settingsfile\r\n");
	fprintf(file, "# Note: This file is automatically generated\r\n\r\n");
	fprintf(file, "# Main Settings\r\n\r\n");
	fprintf(file, "Language = %d\r\n", Language);
	fprintf(file, "Revision = %d\r\n", Revision);
	fprintf(file, "UserAgent = %d\r\n", UserAgent);
	fprintf(file, "Autoupdate = %d\r\n", Autoupdate);
	fprintf(file, "ShowTooltip = %d\r\n", ShowTooltip);
	fprintf(file, "Music = %d\r\n", Music);
	fprintf(file, "DefaultFolder = %s\r\n", DefaultFolder);
	fprintf(file, "Homepage = %s\r\n", Homepage);

	fprintf(file, "\r\n# Favorites\r\n\r\n");
	for(int i = 0; i < N; i++)
        fprintf(file, "Favorite(%d) = %s\r\n", i, Favorites[i]);

    fclose(file);
    return true;
}

bool SSettings::FindConfig()
{
    #ifdef DEBUG
    save_mem("FIND CONFIG");
    #endif
    bool found = false;

    for(int i = SD; i <= USB; i++)
    {
        if(!found)
        {
            snprintf(BootDevice, sizeof(BootDevice), "%s:/", DeviceName[i]);
            snprintf(ConfigPath, sizeof(ConfigPath), "%s:/apps/WiiBrowser/%s", DeviceName[i], CONFIGNAME);
            found = CheckFile(ConfigPath);
        }
        if(!found)
        {
            snprintf(BootDevice, sizeof(BootDevice), "%s:/", DeviceName[i]);
            snprintf(ConfigPath, sizeof(ConfigPath), "%s:/%s%s", DeviceName[i], CONFIGPATH, CONFIGNAME);
            found = CheckFile(ConfigPath);
        }

        #ifdef DEBUG
        save_mem("ConfigPath:");
        save_mem(ConfigPath);
        #endif
    }

    if(!found)
    {
        #ifdef DEBUG
        save_mem("NOT FOUND");
        #endif
        //! No existing config so try to find a place where we can write it too
        for(int i = SD; i <= USB; i++)
        {
            if(!found)
            {
                snprintf(BootDevice, sizeof(BootDevice), "%s:/", DeviceName[i]);
                snprintf(ConfigPath, sizeof(ConfigPath), "%s:/apps/WiiBrowser/%s", DeviceName[i], CONFIGNAME);
                found = IsWritable(ConfigPath);
            }
            if(!found)
            {
                snprintf(BootDevice, sizeof(BootDevice), "%s:/", DeviceName[i]);
                snprintf(ConfigPath, sizeof(ConfigPath), "%s:/%s%s", DeviceName[i], CONFIGPATH, CONFIGNAME);
                found = IsWritable(ConfigPath);
            }
        }
    }

    sprintf(AppPath, "%s%s", BootDevice, DEFAULT_APP_PATH);
    #ifdef DEBUG
    save_mem("AppPath:");
    save_mem(AppPath);
    #endif

    return found;
}

bool SSettings::Load()
{
    #ifdef DEBUG
    save_mem("LOAD");
    char buf[256];
    getcwd(buf, 256);
    save_mem(buf);
    #endif

    if(!FindConfig())
        return false;

    char line[1024];
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s", ConfigPath);

    if(!CheckIntegrity(filepath))
    {
        #ifdef DEBUG
        save_mem("BROKEN");
        #endif

        this->Save();
        return false;
    }

	file = fopen(filepath, "r");
	if (!file)
	{
        fclose(file);
        return false;
	}

	while (fgets(line, sizeof(line), file))
	{
		if (line[0] == '#') continue;

        this->ParseLine(line);
	}
	fclose(file);

    #ifdef DEBUG
    save_mem("LOADED");
    #endif

	return true;
}

bool SSettings::Reset()
{
    this->SetDefault();

    if(this->Save())
        return true;

	return false;
}

bool SSettings::SetSetting(char *name, char *value)
{
    int i = 0;

    if (strcmp(name, "Language") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			Language = i;
		}
		return true;
	}
	if (strcmp(name, "Revision") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			Revision = i;
		}
		return true;
	}
	if (strcmp(name, "UserAgent") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			UserAgent = i;
		}
		return true;
	}
    else if (strcmp(name, "Autoupdate") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			Autoupdate = i;
		}
		return true;
	}
	else if (strcmp(name, "ShowTooltip") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			ShowTooltip = i;
		}
		return true;
	}
	else if (strcmp(name, "Music") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			Music = i;
		}
		return true;
	}
    else if (strcmp(name, "Homepage") == 0) {
        strncpy(Homepage, value, sizeof(Homepage));
		return true;
	}
    else if (strncmp(name, "Favorite", 8) == 0) {
        if (sscanf(name, "Favorite(%d)", &i) == 1) {
            strncpy(Favorites[i], value, 256);
        }
		return true;
	}
    else if (strcmp(name, "DefaultFolder") == 0) {
	    snprintf(DefaultFolder, sizeof(DefaultFolder), value);
	    this->ChangeFolder();
		return true;
	}

    return false;
}

void SSettings::ParseLine(char *line)
{
    char temp[1024], name[1024], value[1024];
    strncpy(temp, line, sizeof(temp));

    char * eq = strchr(temp, '=');
    if(!eq)
        return;
    *eq = 0;

    this->TrimLine(name, temp, sizeof(name));
    this->TrimLine(value, eq+1, sizeof(value));

	this->SetSetting(name, value);
}

void SSettings::TrimLine(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ') src++;
	len = strlen(src);
	while (len > 0 && strchr(" \r\n", src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
}

int SSettings::CheckFolder(const char *folder)
{
    if(folder[0] == '/')
        return RELATIVE;

    for(int i = SD; i <= USB; i++)
    {
        if(!strncmp(folder, DeviceName[i], strlen(DeviceName[i])))
            return ABSOLUTE;
    }

    return FILENAME;
}

bool SSettings::CheckFile(const char *path)
{
    bool found = false;
    FILE * file = fopen(path, "r");
    found = (file != NULL);
    fclose(file);
    return found;
}

bool SSettings::IsWritable(const char *path)
{
    FILE * testFile = fopen(path, "wb");
    bool found = (testFile != NULL);
    fclose(testFile);
    return found;
}

bool SSettings::CheckIntegrity(const char *path)
{
    bool found = false;
    char line[100];
    FILE * file = fopen(path, "r");
    if(file != NULL)
    {
        fgets(line, sizeof(line), file);
        if (!strncmp(line, "APPVERSION",10))
            sscanf(line, "APPVERSION: R%d", &Revision);
        if (!strncmp(line, "# WiiBrowser Settingsfile",25))
            found = true;
    }
    fclose(file);
    return found;
}

char *SSettings::GetUrl(int f)
{
    if(f < 0 || f >= N)
        return NULL;
    return Favorites[f];
}

int SSettings::FindUrl(char *url)
{
    for(int i = 0; i < N; i++)
    {
        if (!strcmp(Favorites[i], url))
            return i;
    }
    return -1;
}

void SSettings::ChangeFolder()
{
    int absolutePath = CheckFolder(DefaultFolder);

    if(absolutePath != ABSOLUTE)
        snprintf(UserFolder, sizeof(UserFolder), "%s%s", BootDevice, DefaultFolder + absolutePath);
    else strncpy(UserFolder, DefaultFolder, sizeof(UserFolder));
}
