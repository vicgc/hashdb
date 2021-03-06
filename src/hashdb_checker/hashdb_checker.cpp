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
 * Provides the main entry for the hashdb_checker tool that exists
 * to provide dfxml query services and to test hashdb interfaces.
 */

#include <config.h>
#ifdef WIN32
  // including winsock2.h now keeps an included header somewhere from
  // including windows.h first, resulting in a warning.
  #include <winsock2.h>
#endif
// Standard includes
#include <cstdlib>
#include <cstdio>
#include <string>
#include "hashdb.hpp"
#include "dfxml_hashdigest_reader.hpp"
#include "hash_query_consumer.hpp"
#include "identified_blocks_reader.hpp"
//#include "md5.h"
#include <getopt.h>

void do_query_hash_md5(hashdb::query_type_t query_type,
                       const std::string& query_path,
                       const std::string& dfxml_infile);

void do_query_source_md5(hashdb::query_type_t query_type,
                         const std::string& query_path,
                         const std::string& identified_blocks_infile);

void do_query_hashdb_info(hashdb::query_type_t query_type,
                         const std::string& query_path);

static std::string see_usage = "Please type 'hashdb_checker -h' for usage.";

// options
bool has_query_type = false;
bool has_client_hashdb_path = false;
bool has_client_socket_endpoint = false;
std::string client_hashdb_path = "hashdb path not defined";
std::string client_socket_endpoint = "tcp://localhost:14500";

void usage() {
  std::cout
  << "hashdb_checker version " << PACKAGE_VERSION  << "\n"
  << "Usage: hashdb_checker -h | -v | <command>\n"
  << "    -h, --help    print this message\n"
  << "    --Version     print version number\n"
  << "\n"
  << "hashdb_checker supports the following <command> options:\n"
  << "\n"
  << "query_hash [<query parameter>]+ <dfxml input>\n"
  << "    Queries the hashdb specified by <query parameter> for hash values\n"
  << "    that match hash values in <dfxml input>.\n"
  << "\n"
  << "    Options:\n"
  << "    <query parameter>\n"
  << "        Please see <query parameter> options.\n"
  << "\n"
  << "    Parameters:\n"
  << "        <dfxml input>  a DFXML file containing hashes to be looked up\n"
  << "\n"
  << "query_source [<query parameter>]+ <identified_blocks.txt input>\n"
  << "    Queries the hashdb specified by <query parameter> for the repository\n"
  << "    name, filename, and file offset of where the hash values specified\n"
  << "    in <identified_blocks.txt input> are sourced.\n"
  << "\n"
  << "    Options:\n"
  << "    <query parameter>\n"
  << "        Please see <query parameter> options.\n"
  << "\n"
  << "    Parameters:\n"
  << "        <identified_blocks.txt input> an identified_blocks.txt feature\n"
  << "        file created by bulk_extractor that contains hashes whose\n"
  << "        sources are to be looked up.\n"
  << "\n"
  << "query_hashdb_info [<query parameter>]+\n"
  << "    Queries the hashdb specified by <query parameter> for information about\n"
  << "    the hashdb.\n"
  << "\n"
  << "    Options:\n"
  << "    <query parameter>\n"
  << "        Please see <query parameter> options.\n"
  << "\n"
  << "<query parameter> options establish the query type and location:\n"
  << "    -q, --query_type=<query type>\n"
  << "        <query type> used to perform the query, where <query_type>\n"
  << "        is one of use_path | use_socket (default use_path).\n"
  << "        use_path   - Lookups are performed from a hashdb in the filesystem\n"
  << "                     at the specified <path>.\n"
  << "        use_socket - Lookups are performed from a server service at the\n"
  << "                     specified <socket>.\n"
  << "\n"
  << "    -p, --path=<path>\n"
  << "        specifies the <path> to the hash database to be used for performing\n"
  << "        the query service. This option is only valid when the query type\n"
  << "        is set to \"use_path\".\n"
  << "\n"
  << "    -s, --socket=<socket>\n"
  << "        specifies the client <socket> endpoint to use to connect with the\n"
  << "        hashdb server (default '" << client_socket_endpoint << "').  Valid socket\n"
  << "        transports supported by the zmq messaging kernel are tcp, ipc, and\n"
  << "        inproc.  Currently, only tcp is tested.  This option is only valid\n"
  << "        when the query type is set to \"use_socket\".\n"
  << "\n"
  ;
}

int main(int argc,char **argv)
{

  // defaults
  hashdb::query_type_t query_type = hashdb::QUERY_USE_PATH;

  // manage when there are no arguments
  if(argc==1) {
      usage();
      exit(1);
  }

  // parse options
  int option_index; // not used
  while (1) {

    static struct option long_options[] = {
      // general
      {"help", no_argument, 0, 'h'},
      {"Version", no_argument, 0, 'V'},

      // command options
      {"query_type", required_argument, 0, 'l'},
      {"path", required_argument, 0, 'p'},
      {"socket", required_argument, 0, 's'},

      {0,0,0,0}
    };

    int ch = getopt_long(argc, argv, "hVq:p:s:", long_options, &option_index);
    if (ch == -1) {
      // no more arguments
      break;
    }
    if (ch == 0) {
      // command options set flags and use ch==0
      continue;
    }
    switch (ch) {
      case 'h': {	// help
        usage();
        exit(0);
        break;
      }
      case 'V': {	// Version
        std::cout << "hashdb_checker " << PACKAGE_VERSION << "\n";
        exit(0);
        break;
      }
      case 'q': {	// query type
        has_query_type = true;
        bool is_ok_query_type = string_to_query_type(optarg, query_type);
        if (!is_ok_query_type) {
          std::cerr << "Invalid query type: '" << optarg << "'.  " << see_usage << "\n";
          exit(1);
        }
        break;
      }
      case 'p': {	// client hashdb path
        has_client_hashdb_path = true;
        client_hashdb_path = optarg;
        break;
      }
      case 's': {	// client socket endpoint
        has_client_socket_endpoint = true;
        client_socket_endpoint = optarg;
        break;
      }
      default:
//        std::cerr << "unexpected command character " << ch << "\n";
        exit(1);
    }
  }

  // set query path based on query type
  std::string query_path;
  switch(query_type) {
    case hashdb::QUERY_USE_PATH:   query_path = client_hashdb_path;     break;
    case hashdb::QUERY_USE_SOCKET: query_path = client_socket_endpoint; break;
    default:            query_path = "query path not defined";
  }

  // if the query type is QUERY_USE_PATH then the query path must be provided
  if (query_type == hashdb::QUERY_USE_PATH && has_client_hashdb_path == false) {
    std::cerr << "The --path parameter is required when the query type is 'query_path'\n";
    exit(1);
  }

  // now parse tokens that are not consumed by options
  argc -= optind;
  argv += optind;

  // get the command
  if (argc < 1) {
    std::cerr << "Error: a command must be provided.\n";
    exit(1);
  }
  const std::string command(argv[0]);
  argc--;
  argv++;

  // get any arguments
  const int num_args = argc;
  const std::string arg1((argc>=1) ? argv[0] : "");
  const std::string arg2((argc>=2) ? argv[1] : "");

  // available commands
  const std::string COMMAND_QUERY_HASHDB_INFO("query_hashdb_info");
  const std::string COMMAND_QUERY_HASH("query_hash");
  const std::string COMMAND_QUERY_SOURCE("query_source");

  // run the command

  // query hashdb info
  if (command == COMMAND_QUERY_HASHDB_INFO) {
    if (num_args != 0) {
      std::cerr << "The query hashdb info command requires that there be no additional parameters.  " << see_usage << "\n";
      exit(1);
    }
 
    do_query_hashdb_info(query_type, query_path);

  // query hash
  } else if (command == COMMAND_QUERY_HASH) {
    if (num_args != 1) {
      std::cerr << "The query_hash command requires 1 parameter.  " << see_usage << "\n";
      exit(1);
    }
 
    if (has_client_hashdb_path && has_client_socket_endpoint) {
      std::cerr << "A path or a socket may be selected, but not both.  " << see_usage << "\n";
      exit(1);
    }
    do_query_hash_md5(query_type, query_path, arg1);

  // query source
  } else if (command == COMMAND_QUERY_SOURCE) {
    if (num_args != 1) {
      std::cerr << "The query_source command requires 1 parameter.  " << see_usage << "\n";
      exit(1);
    }
 
    if (has_client_hashdb_path && has_client_socket_endpoint) {
      std::cerr << "A path or a socket may be selected, but not both.  " << see_usage << "\n";
      exit(1);
    }
    do_query_source_md5(query_type, query_path, arg1);

  } else {
    // program error
    assert(0);
  }

  // done
  return 0;
}

void do_query_hash_md5(hashdb::query_type_t query_type,
                       const std::string& query_path,
                       const std::string& dfxml_infile) {
  std::cout << "hashdb query, query type '" << query_type_to_string(query_type)
            << "', query path '" << query_path << "'\n";

  // request, response, and source text map
  hashdb::hashes_request_md5_t request;
  hashdb::hashes_response_md5_t response;
  std::map<uint32_t, std::string> source_map;

  // create the hash query consumer with its consume callback method
  hash_query_consumer_t hash_query_consumer(&request, &source_map);

  // perform the DFXML read
  dfxml_hashdigest_reader_t<hash_query_consumer_t>::do_read(
                             dfxml_infile, "not used", &hash_query_consumer);

  // create the client query service
  hashdb::query_t query(query_type, query_path);

  // verify that the query source opened
  if (query.query_status() != 0) {
    std::cerr << "Unable to open query service for hash query.  Aborting.\n";
    exit(1);
  }

  // perform the query
  int status = query.query_hashes_md5(request, response);

  // show result
  if (status == 0) {
    for (std::vector<hashdb::hash_response_md5_t>::const_iterator it = response.begin(); it != response.end(); ++it) {

      const uint8_t* digest = it->digest;
      md5_t md5;
      memcpy(md5.digest, digest, 16);

      std::cout << source_map[it->id] << "\t"
                << md5 << "\t"
                << "count=" << it->duplicates_count
                << ",source_query_index=" << it->source_query_index
                << ",hash_block_offset_value=" << it->hash_block_offset_value
                << "\n";
    }
  } else {
    std::cerr << "Failure in accessing the hashdb for hash query.  Aborting.\n";
    exit(1);
  }
}

void do_query_source_md5(hashdb::query_type_t query_type,
                         const std::string& query_path,
                         const std::string& identified_blocks_infile) {
  std::cout << "hashdb query, query type '" << query_type_to_string(query_type)
            << "', query path '" << query_path << "'\n";

  // request, response, and offset map
  hashdb::sources_request_md5_t request;
  hashdb::sources_response_md5_t response;
  std::map<uint32_t, std::string> offset_map;

  // read the identified blocks
  identified_blocks_reader_t(identified_blocks_infile, request, offset_map);

  // create the client query service
  hashdb::query_t query(query_type, query_path);

  // verify that the query source opened
  if (query.query_status() != 0) {
    std::cerr << "Unable to open query service for source query.  Aborting.\n";
    exit(1);
  }

  // perform the query
  int status = query.query_sources_md5(request, response);

  // show result
  if (status == 0) {
    for (hashdb::sources_response_md5_t::const_iterator it = response.begin(); it != response.end(); ++it) {

      // get offset and md5 digest together
      std::string offset = offset_map[it->id];
      const uint8_t* digest = it->digest;
      md5_t md5;
      memcpy(md5.digest, digest, 16);

      // output one line for each source reference
      for (hashdb::source_references_t::const_iterator source_reference_it = it->source_references.begin(); source_reference_it != it->source_references.end(); ++source_reference_it) {

        std::cout << offset << "\t"
                  << md5 << "\t"
                  << "repository_name=" << source_reference_it->repository_name
                  << ",filename=" << source_reference_it->filename
                  << ",file_offset=" << source_reference_it->file_offset
                  << "\n";
      }
    }
  } else {
    std::cerr << "Failure in accessing the hashdb server for source query.\n";
    exit(1);
  }
}

void do_query_hashdb_info(hashdb::query_type_t query_type,
                          const std::string& query_path) {
  std::cout << "hashdb info, query type '" << query_type_to_string(query_type)
            << "', query path '" << query_path << "'\n";

  // allocate space for the response
  std::string response;

  // create the client query service
  hashdb::query_t query(query_type, query_path);

  // perform the information query
  int status = query.query_hashdb_info(response);
  if (status == 0) {
    std::cout << "hashdb info:\n"
              << response
              << "\n"
              ;
  } else {
    std::cerr << "Failure in accessing the hashdb server for info query.\n";
    exit(1);
  }
}

