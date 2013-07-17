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
#include <mxml.h>

#include <ogcsys.h>
#include <common.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "Settings.h"
#include "menu.h"

#define DEFAULT_APP_PATH    "apps/wiibrowser/"
#define DEFAULT_HOMEPAGE    "www.google.com/"
#define CONFIGPATH          "apps/wiibrowser/"
#define CONFIGNAME          "wiibrowser.cfg"

void *LoadFile(char *filepath, int size);
void WriteFile(char *filepath, int size, void *buffer);
static unsigned int GetFileSize(const char *filename);

static mxml_node_t *xml = NULL;
static mxml_node_t *node = NULL;

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
    Autoupdate = STABLE;

    ShowTooltip = true;
    Restore = true;
    ShowThumbs = true;
    ExecLua = true;
    CleanExit = true;

    MuteSound = false;
    IFrame = false;
    DocWrite = false;

    sprintf(Homepage, DEFAULT_HOMEPAGE);
    sprintf(DefaultFolder, DEFAULT_APP_PATH);

    sprintf(AppPath, "%s%s", BootDevice, DEFAULT_APP_PATH);
    sprintf(UserFolder, AppPath);

    sprintf(Uuid, "WIIB-00000000");
    sprintf(Revision, "Rev000");
    sscanf(Revision, "Rev%d", &RevInt);

    for(int i = 0; i < N; i++)
    {
        TopSites[i] = new char[512];
        memset(TopSites[i], 0, 512);
        Thumbnails[i] = NULL;
    }

    memset(Proxy, 0, 256);
    memset(StartPage, 0, 256);

    Favorites = NULL;
    num_fav = 0;
}

bool SSettings::Save(bool clean)
{
    if(!FindConfig())
        return false;

    char filedest[100];
    snprintf(filedest, sizeof(filedest), "%s", ConfigPath);

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
	fprintf(file, "Revision = %d\r\n", RevInt);
	fprintf(file, "Autoupdate = %d\r\n", Autoupdate);
	fprintf(file, "MuteSound = %d\r\n", MuteSound);
	fprintf(file, "ShowTooltip = %d\r\n", ShowTooltip);
	fprintf(file, "ShowThumbnails = %d\r\n", ShowThumbs);
	fprintf(file, "RestoreSession = %d\r\n", Restore);
	fprintf(file, "DefaultFolder = %s\r\n", DefaultFolder);
	fprintf(file, "Homepage = %s\r\n", Homepage);

	fprintf(file, "\r\n# Top Sites\r\n\r\n");
	for(int i = 0; i < N; i++)
        fprintf(file, "Favorite(%d) = %s\r\n", i, TopSites[i]);

    fprintf(file, "\r\n# Advanced\r\n\r\n");
	fprintf(file, "UserAgent = %d\r\n", UserAgent);
	fprintf(file, "IFrame = %d\r\n", IFrame);
	fprintf(file, "DocWrite = %d\r\n", DocWrite);
	fprintf(file, "ExecLua = %d\r\n", ExecLua);
	fprintf(file, "CleanExit = %d\r\n", clean);
	fprintf(file, "Proxy = %s\r\n", Proxy);
    fclose(file);

    // save thumbnails
    snprintf(filedest, sizeof(filedest), "%s/thumbnails", AppPath);

    DIR *dir = opendir(filedest);
    int size = 640*480*4;

    if(!dir && mkdir(filedest, 0777) != 0)
        return false;
    else closedir(dir);

    for (int i = 0; i < N; i++)
    {
        snprintf(filedest, sizeof(filedest), "%s/thumbnails/thumb_%d.gxt", AppPath, i);
        if (!Thumbnails[i])
            continue;

        WriteFile(filedest, size, Thumbnails[i]);
    }

    SaveFavorites();
    return true;
}

bool SSettings::FindConfig()
{
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
    }

    if(!found)
    {
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
    return found;
}

bool SSettings::Load()
{
    if(!FindConfig())
        return false;

    char line[1024];
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s", ConfigPath);

    if(!CheckIntegrity(filepath))
    {
        this->Save(0);
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

    int size = 640*480*4;

    // load thumbnails
    for (int i = 0; i < N; i++)
    {
        snprintf(filepath, sizeof(filepath), "%s/thumbnails/thumb_%d.gxt", AppPath, i);

        if (TopSites[i][0])
            Thumbnails[i] = (u8 *)LoadFile(filepath, size);
        else remove(filepath);
    }

    LoadFavorites();
    return true;
}

bool SSettings::Reset()
{
    this->SetDefault();

    if(this->Save(0))
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
	else if (strcmp(name, "UserAgent") == 0) {
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
	else if (strcmp(name, "MuteSound") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			MuteSound = i;
		}
		return true;
	}
    else if (strcmp(name, "ShowTooltip") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			ShowTooltip = i;
		}
		return true;
	}
    else if (strcmp(name, "ShowThumbnails") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			ShowThumbs = i;
		}
		return true;
	}
    else if (strcmp(name, "RestoreSession") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			Restore = i;
		}
		return true;
	}
    else if (strcmp(name, "Homepage") == 0) {
        strncpy(Homepage, value, sizeof(Homepage));
		return true;
	}
    else if (strcmp(name, "Proxy") == 0) {
        strncpy(Proxy, value, sizeof(Proxy));
		return true;
	}
    else if (strncmp(name, "Favorite", 8) == 0) {
        if (sscanf(name, "Favorite(%d)", &i) == 1) {
            strncpy(TopSites[i], value, 512);
        }
		return true;
	}
    else if (strcmp(name, "DefaultFolder") == 0) {
	    snprintf(DefaultFolder, sizeof(DefaultFolder), value);
	    this->ChangeFolder();
		return true;
	}
    else if (strcmp(name, "IFrame") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			IFrame = i;
		}
		return true;
	}
    else if (strcmp(name, "DocWrite") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			DocWrite = i;
		}
		return true;
	}
    else if (strcmp(name, "ExecLua") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			ExecLua = i;
		}
		return true;
	}
    else if (strcmp(name, "CleanExit") == 0) {
		if (sscanf(value, "%d", &i) == 1) {
			CleanExit = i;
		}
		return true;
	}

    return false;
}

void SSettings::ParseLine(char *line)
{
    char temp[1024], name[1024], value[1024];
    strncpy(temp, line, sizeof(temp));

    char *eq = strchr(temp, '=');
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
            sscanf(line, "APPVERSION: R%d", &RevInt);
        if (!strncmp(line, "# WiiBrowser Settingsfile",25))
            found = true;
    }
    fclose(file);
    return found;
}

struct favorite *SSettings::GetFav(int f)
{
    if(f < 0 || f >= num_fav-1)
        return NULL;
    return &Favorites[f];
}

char *SSettings::GetUrl(int f)
{
    if(f < 0 || f >= N)
        return NULL;
    return TopSites[f];
}

int SSettings::FindUrl(char *url)
{
    if(!url || !url[0])
        return -1;

    for(int i = 0; i < N; i++)
    {
        if (!strcmp(TopSites[i], url))
            return i;
    }
    return -1;
}

void SSettings::Remove(int f, bool update)
{
    if(f < 0 || f >= N)
        return;

    if(!update)
        bzero(TopSites[f], 512);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/thumbnails/thumb_%d.gxt", AppPath, f);

    if(Thumbnails[f])
    {
        free(Thumbnails[f]);
        Thumbnails[f] = NULL;
        remove(filepath);
    }
}

void SSettings::ChangeFolder()
{
    int absolutePath = CheckFolder(DefaultFolder);

    if(absolutePath != ABSOLUTE)
        snprintf(UserFolder, sizeof(UserFolder), "%s%s", BootDevice, DefaultFolder + absolutePath);
    else strncpy(UserFolder, DefaultFolder, sizeof(UserFolder));
}

void SSettings::SetStartPage(char *page)
{
    int i = 0;
    if(!strcasecmp(page, "homepage"))
        strcpy(StartPage, Homepage);
    else if(!sscanf(page, "bookmark_%d", &i))
        strncpy(StartPage, page, sizeof(StartPage));
    else if(GetUrl(i))
        strcpy(StartPage, GetUrl(i));
}

int SSettings::GetStartPage(char *dest)
{
    if(!strlen(StartPage))
        return MENU_HOME;

    strcpy(dest, StartPage);
    return MENU_BROWSE;
}

extern string ParseList(char *buffer);

bool SSettings::LoadFavorites()
{
    // load favorites
    char filedest[100];
    snprintf(filedest, sizeof(filedest), "%s/appdata/bookmarks.html", AppPath);
    char *string = (char*)LoadFile(filedest, GetFileSize(filedest));

    if (!string)
    {
        CreateXMLFile();
        return false;
    }

    const char *parse = ParseList(string).c_str();
    char *ptr = strstr(parse, "<?xml");
    xml = mxmlLoadString(NULL, ptr, MXML_OPAQUE_CALLBACK);

    if(!xml || !ptr)
    {
        CreateXMLFile();
        return false;
    }

    num_fav = 1;

    for (node = mxmlFindElement(xml, xml, "a", "href", NULL, MXML_DESCEND);
            node != NULL;
            node = mxmlFindElement(node, xml, "a", "href", NULL, MXML_DESCEND))
    {
        const char * tmp = mxmlElementGetAttr(node, "href");
        const char * name = node->child->value.opaque;

		if(tmp)
		{
            Favorites = (struct favorite*)realloc(Favorites, num_fav*sizeof(struct favorite));
            strncpy(Favorites[num_fav-1].url, tmp, 512);

            if(name)
                strncpy(Favorites[num_fav-1].name, name, 512);
            num_fav++;
		}
    }

	free(string);
    return true;
}

int SSettings::IsBookmarked(char *url)
{
    if(!url || !url[0])
        return -1;

    for(int i = 0; i < num_fav-1; i++)
    {
        if (!strcmp(Favorites[i].url, url))
            return i;
    }
    return -1;
}

void SSettings::AddFavorite(char *url, char *title)
{
    Favorites = (struct favorite*)realloc(Favorites, num_fav*sizeof(struct favorite));
    strncpy(Favorites[num_fav-1].url, url, 512);
    strncpy(Favorites[num_fav-1].name, title, 512);

    node = mxmlFindElement(xml, xml, "dl", NULL, NULL, MXML_DESCEND);
    if(!node)
        node = mxmlNewElement(xml, "dl");

    node = mxmlNewElement(node, "dt");
    node = mxmlNewElement(node, "a");
    mxmlElementSetAttr(node, "href", url);

    if(title)
        mxmlNewText(node, 0, title);
    else mxmlNewText(node, 0, "");

    num_fav++;
}

void SSettings::DelFavorite(char *url)
{
    int ind = IsBookmarked(url);

    if(ind >= 0)
    {
        for(int i = ind; i<num_fav-2; i++)
            Favorites[i] = Favorites[i+1];

        Favorites = (struct favorite*)realloc(Favorites, (num_fav-2)*sizeof(struct favorite));
        num_fav--;

        node = mxmlFindElement(xml, xml, "a", "href", url, MXML_DESCEND);
        if(node)
            mxmlDelete(mxmlGetParent(node));
    }
}

bool SSettings::SaveFavorites()
{
    // save favorites
    char filedest[100];
    snprintf(filedest, sizeof(filedest), "%s/appdata", AppPath);
    DIR *dir = opendir(filedest);

    if(!dir && mkdir(filedest, 0777) != 0)
        return false;
    else closedir(dir);

    snprintf(filedest, sizeof(filedest), "%s/appdata/bookmarks.html", AppPath);
    FILE *file = fopen(filedest, "w");

    if(xml)
    {
        fputs("<!DOCTYPE NETSCAPE-Bookmark-file-1>", file);
        char *ptr = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
        fputs(ptr, file);
        free(ptr);
    }

    fclose(file);
    // mxmlDelete(xml);
    return true;
}

bool SSettings::CreateXMLFile()
{
    xml = mxmlNewXML("1.0");
	mxmlSetWrapMargin(0); // disable line wrapping

    node = mxmlNewElement(xml, "meta");
    mxmlElementSetAttr(node, "http-equiv", "Content-Type");
    mxmlElementSetAttr(node, "content", "text/html; charset=UTF-8");

    node = mxmlNewElement(xml, "title");
    mxmlNewText(node, 0, "Bookmarks");

    node = mxmlNewElement(xml, "dl");

    return true;
}

static unsigned int
GetFileSize(const char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) != 0)
    {
        return 0;
    }
    return sb.st_size;
}

void *LoadFile(char *filepath, int size)
{
    FILE *file = fopen(filepath, "rb");
    if(!file)
    {
        fclose(file);
        return NULL;
    }

    u8 *buffer = (u8 *)malloc(size);
    if(!buffer)
        return NULL;

    fread(buffer, size, 1, file);
    fclose(file);

    return buffer;
}

void WriteFile(char *filepath, int size, void *buffer)
{
    FILE *file = fopen(filepath, "wb");
    if(!file)
    {
        fclose(file);
        return;
    }

    fwrite(buffer, size, 1, file);
    fclose(file);
}
