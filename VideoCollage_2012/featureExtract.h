/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
Video Analysis Sdk

Abstract:
This header file provide the functions for feature extract

Notes:


History:
Created on 08/02/2007 by v-huami@microsoft.com
Updated on 10/24/2013 by v-aotang@microsoft.com
\******************************************************************************/

#pragma once
#include<string>

struct ExtractConfig;
struct MetaDataExtractConfig;

void SaveAll(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool AllFeatureExtract(const ExtractConfig & config);
bool AllFeatureExtractHelper_File(const ExtractConfig & config);
bool AllFeatureExtractHelper_Dir(const ExtractConfig & config);
bool ExtractMotionThumbnail(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
void ExtractScene(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
void ExtractSubshot(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool  CreateSavedFloders(const ExtractConfig & config);
bool ConfigRegionFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool ConfigGlobalFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool ConfigStructureFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool ConfigThumbnailExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig);
bool VideoInfoSave(const std::wstring & srcFile, const std::wstring & dstDir, const MetaDataExtractConfig & extConfig);
