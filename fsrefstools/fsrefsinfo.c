/*
 * Shows information obtained from a Windows Resiliant File System (REFS)
 *
 * Copyright (C) 2012-2016, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <file_stream.h>
#include <memory.h>
#include <types.h>

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#include "fsrefsoutput.h"
#include "fsrefstools_libcerror.h"
#include "fsrefstools_libclocale.h"
#include "fsrefstools_libcnotify.h"
#include "fsrefstools_libcsystem.h"
#include "fsrefstools_libfsrefs.h"
#include "info_handle.h"

info_handle_t *fsrefsinfo_info_handle = NULL;
int fsrefsinfo_abort                  = 0;

/* Prints the executable usage information
 */
void usage_fprint(
      FILE *stream )
{
	if( stream == NULL )
	{
		return;
	}
	fprintf( stream, "Use fsrefsinfo to determine information about a Windows\n"
	                 " Resiliant File System (REFS).\n\n" );

	fprintf( stream, "Usage: fsrefsinfo [ -e entry_index ] [ -o offset ]\n"
	                 "                  [ -hvV ] source\n\n" );

	fprintf( stream, "\tsource: the source file or device\n\n" );

	fprintf( stream, "\t-e:     show info about a specific MFT entry index\n"
	                 "\t        (default = 5)\n" );
	fprintf( stream, "\t-h:     shows this help\n" );
	fprintf( stream, "\t-o:     specify the volume offset\n" );
	fprintf( stream, "\t-v:     verbose output to stderr\n" );
	fprintf( stream, "\t-V:     print version\n" );
}

/* Signal handler for fsrefsinfo
 */
void fsrefsinfo_signal_handler(
      libcsystem_signal_t signal LIBCSYSTEM_ATTRIBUTE_UNUSED )
{
	libcerror_error_t *error = NULL;
	static char *function   = "fsrefsinfo_signal_handler";

	LIBCSYSTEM_UNREFERENCED_PARAMETER( signal )

	fsrefsinfo_abort = 1;

	if( fsrefsinfo_info_handle != NULL )
	{
		if( info_handle_signal_abort(
		     fsrefsinfo_info_handle,
		     &error ) != 1 )
		{
			libcnotify_printf(
			 "%s: unable to signal info handle to abort.\n",
			 function );

			libcnotify_print_error_backtrace(
			 error );
			libcerror_error_free(
			 &error );
		}
	}
	/* Force stdin to close otherwise any function reading it will remain blocked
	 */
	if( libcsystem_file_io_close(
	     0 ) != 0 )
	{
		libcnotify_printf(
		 "%s: unable to close stdin.\n",
		 function );
	}
}

/* The main program
 */
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	libfsrefs_error_t *error                            = NULL;
	libcstring_system_character_t *option_entry_index   = NULL;
	libcstring_system_character_t *option_volume_offset = NULL;
	libcstring_system_character_t *source               = NULL;
	char *program                                       = "fsrefsinfo";
	libcstring_system_integer_t option                  = 0;
	int verbose                                         = 0;

	libcnotify_stream_set(
	 stderr,
	 NULL );
	libcnotify_verbose_set(
	 1 );

	if( libclocale_initialize(
             "fsrefstools",
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize locale values.\n" );

		goto on_error;
	}
        if( libcsystem_initialize(
             _IONBF,
             &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize system values.\n" );

		goto on_error;
	}
	fsrefsoutput_version_fprint(
	 stdout,
	 program );

	while( ( option = libcsystem_getopt(
	                   argc,
	                   argv,
	                   _LIBCSTRING_SYSTEM_STRING( "e:ho:vV" ) ) ) != (libcstring_system_integer_t) -1 )
	{
		switch( option )
		{
			case (libcstring_system_integer_t) '?':
			default:
				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_LIBCSTRING_SYSTEM "\n",
				 argv[ optind - 1 ] );

				usage_fprint(
				 stdout );

				return( EXIT_FAILURE );

			case (libcstring_system_integer_t) 'e':
				option_entry_index = optarg;

				break;

			case (libcstring_system_integer_t) 'h':
				usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'o':
				option_volume_offset = optarg;

				break;

			case (libcstring_system_integer_t) 'v':
				verbose = 1;

				break;

			case (libcstring_system_integer_t) 'V':
				fsrefsoutput_copyright_fprint(
				 stdout );

				return( EXIT_SUCCESS );
		}
	}
	if( optind == argc )
	{
		fprintf(
		 stderr,
		 "Missing source file or device.\n" );

		usage_fprint(
		 stdout );

		return( EXIT_FAILURE );
	}
	source = argv[ optind ];

	libcnotify_verbose_set(
	 verbose );
	libfsrefs_notify_set_stream(
	 stderr,
	 NULL );
	libfsrefs_notify_set_verbose(
	 verbose );

	if( info_handle_initialize(
	     &fsrefsinfo_info_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize info handle.\n" );

		goto on_error;
	}
	if( option_entry_index != NULL )
	{
		if( info_handle_set_entry_index(
		     fsrefsinfo_info_handle,
		     option_entry_index,
		     &error ) != 1 )
		{
			libcnotify_print_error_backtrace(
			 error );
			libcerror_error_free(
			 &error );

			fprintf(
			 stderr,
			 "Unsupported entry index defaulting to: %" PRIi64 ".\n",
			 fsrefsinfo_info_handle->entry_index );
		}
	}
	if( option_volume_offset != NULL )
	{
		if( info_handle_set_volume_offset(
		     fsrefsinfo_info_handle,
		     option_volume_offset,
		     &error ) != 1 )
		{
			libcnotify_print_error_backtrace(
			 error );
			libcerror_error_free(
			 &error );

			fprintf(
			 stderr,
			 "Unsupported volume offset defaulting to: %" PRIi64 ".\n",
			 fsrefsinfo_info_handle->volume_offset );
		}
	}
	if( info_handle_open_input(
	     fsrefsinfo_info_handle,
	     source,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to open: %" PRIs_LIBCSTRING_SYSTEM ".\n",
		 source );

		goto on_error;
	}
	if( info_handle_volume_fprint(
	     fsrefsinfo_info_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to print volume information.\n" );

		goto on_error;
	}
	if( info_handle_close_input(
	     fsrefsinfo_info_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close info handle.\n" );

		goto on_error;
	}
	if( info_handle_free(
	     &fsrefsinfo_info_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free info handle.\n" );

		goto on_error;
	}
	return( EXIT_SUCCESS );

on_error:
	if( error != NULL )
	{
		libcnotify_print_error_backtrace(
		 error );
		libcerror_error_free(
		 &error );
	}
	if( fsrefsinfo_info_handle != NULL )
	{
		info_handle_free(
		 &fsrefsinfo_info_handle,
		 NULL );
	}
	return( EXIT_FAILURE );
}

