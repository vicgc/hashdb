// Author:  Bruce Allen <bdallen@nps.edu>
// Created: 2/25/2013
//
// The software provided here is released by the Naval Postgraduate
// School, an agency of the U.S. Department of Navy.  The software
// bears no warranty, either expressed or implied. NPS does not assume
// legal liability nor responsibility for a User's use of the software
// or the results of such use.
//
// Please note that within the United States, copyright protection,
// under Section 105 of the United States Code, Title 17, is not
// available for any work of the United States Government and/or for
// any works created by United States Government employees. User
// acknowledges that this software contains work which was created by
// NPS government employees and is therefore in the public domain and
// not subject to copyright.
//
// Released into the public domain on February 25, 2013 by Bruce Allen.

/**
 * \file
 * Provides hashdb filenames.
 */

#ifndef HASHDB_FILENAMES_HPP
#define HASHDB_FILENAMES_HPP

#include <string>

namespace hashdb_filenames_t {

//std::string metadata_filename(const std::string& hashdb_dir);
 
    std::string settings_filename(const std::string& hashdb_dir);
    std::string log_filename(const std::string& hashdb_dir);
    std::string history_filename(const std::string& hashdb_dir);
    std::string hash_store_filename(const std::string& hashdb_dir);
    std::string hash_duplicates_store_filename(const std::string& hashdb_dir);
    std::string source_lookup_prefix(const std::string& hashdb_dir);
    std::string source_repository_name_prefix(const std::string& hashdb_dir);
    std::string source_filename_prefix(const std::string& hashdb_dir);
    std::string repository_name_filename(const std::string& hashdb_dir);
    std::string bloom1_filename(const std::string& hashdb_dir);
    std::string bloom2_filename(const std::string& hashdb_dir);
}

#endif

