#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "arg.h"
#include "types.h"
#include <limits.h>

static const char PROGRAM_VERSION[] = "0.0.1";

void cleanup_o (argo o) {
	if (NULL != o->out_format) free (o->out_format);
	if (NULL != o->svg_file) free (o->svg_file);
	if (NULL != o->out_file) free (o->out_file);
	if (NULL != o) free (o);
}

void null_options (argo o) {
	// Null out all the options
	o->no_arrows = false;
	o->no_numbers = false;
	o->no_Start_mark = false;
	o->sequential_images = false;
	o->width = -1;
	o->height = -1;
	o->num_of_files = 0;
	o->out_format = NULL;
	o->svg_file = NULL;
	o->out_file = NULL;
}

void help (const char *argv0) {
	
	char *argv0_copy = strdup (argv0);
    char *argv0_base = basename (argv0_copy);
	
	fprintf (stderr, "Usage: %s [OPTIONS] <SVG_file> <out_file>\n", argv0_base);
	puts ("");
	fprintf (stderr, "  -w, --width=WIDTH\tWidth of output image in pixels.\n");
	fprintf (stderr, "  -h, --height=HEIGHT\tHeight of output image in pixels.\n");
	fprintf (stderr, "  -m, --sequence\tSequential image.\n");
	fprintf (stderr, "  -o, --format=FORMAT\tThe desired output file format , svg or png [png is default].\n");
	puts ("");
	fprintf (stderr, "  -a, --no-arrows\tRemove arrows from the output image.\n");
	fprintf (stderr, "  -n, --no-numbers\tRemove numbers from the output.\n");
	fprintf (stderr, "  -s, --no-startmark\tRemove start marks from the output.\n");
	puts ("");
	fprintf (stderr, "  -?, --help\t\tGive this help.\n");
	fprintf (stderr, "  -V, --version\t\tProgram Version.\n");
	free (argv0_copy);
}

void process_args (char **argv, int argc, argo opts) {
	
	int c;
	
	// the long options for the program
	static struct option long_options[] = {

		/* These options set a bool flag. */
		{"version", no_argument, NULL, 'V'}, 
		{"no-arrows", no_argument, NULL, 'a'}, 
		{"no-numbers", no_argument, NULL, 'n'}, 
		{"no-startmark", no_argument, NULL, 's'},  
		{"sequence", no_argument, NULL, 'm'}, 
		{"help", no_argument, NULL, '?'}, 

		/* THese options have an argument requirement */
		{"width", required_argument, NULL, 'w'}, 
		{"format", required_argument, NULL, 'i'}, 
		{"height", required_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	// parse the options and set the values in the options structure
	while ( (c = getopt_long (argc, argv, "V?no:amsw:h:", long_options, NULL)) != -1)
		switch (c) {
			case 'w':
				// Add the argument for the -w tag as an integer to the struct
				opts->width = atoi (optarg);
				break;
			case 'h':
				// Add the argument for the -h tag as an integer to the struct
				opts->height = atoi (optarg);
				break;
			case 'm':
				// Output the strokes in individual images collectively
				opts->sequential_images = true;
				break;
			case 'n':
				// Set the remove numbers option
				opts->no_numbers = true;
				break;
			case 'o':
				// Set the output format.

				/***************************************************/
				/*** NEED TO ADD FORMAT VALIDATION CHECK HERE    ***/
				/***************************************************/

				// Allocate memory for the format string
				opts->out_format = (char*)malloc (strlen (optarg)+2);

				// copy the format string to the allocated memory
				strncpy (opts->out_format, optarg, strlen (optarg));

				// ensure the last char is a '\0' for later string processing
				opts->out_format[strlen (optarg)] = '\0';
				break;
			case 's':
				// Set the remove start mark option
				opts->no_Start_mark = true;
				break;
			case 'a':
				// Set the remove arrows option
				opts->no_arrows = true;
				break;
			case 'V':
				// Output the version of the program option
				fprintf (stderr, "Version v%s\n", PROGRAM_VERSION);
				exit (0);
				break;
			case '?':
				// Output the help information
				help (argv[0]);
				exit (1);
				break;
			default:
				break;
		}
	
	// Add the input file to the args structure
	if (argc - optind >= 1) {

		// Allocate memory for the file name
		opts->svg_file = (char*)malloc (FILENAME_MAX);
		
		// Copy the file name argument to the allocated memory
		strncpy (opts->svg_file, argv[optind++], FILENAME_MAX);
		
		// check for another filename argument
		if (argc - optind >= 1) {

			// Add the output file to the args structure
			// Allocate memory for the file name
			opts->out_file = (char*)malloc (FILENAME_MAX);

			// Copy the file name argument to the allocated memory
			strncpy (opts->out_file, argv[optind++], FILENAME_MAX);

			// check for another filename
			if (argc - optind > 0) {

				// Print out the options info and quit
				help (argv[0]);
				exit (1);
			}
		}
	}

	// Check to make sure we have a file to process and that it is a valid file 
	if ( NULL == opts->svg_file || !file_exists (opts->svg_file) || !is_type (opts->svg_file, ".svg")) {

		// Print error and quit
		printf ("No .svg file to process. Quitting...\n");
		
		// dealloc any memory before quitting
		cleanup_o (opts);
		exit (0);
		}

	// check to see if no output format has been set.
	if (NULL == opts->out_format) {
		
		// check to see if no output filename has been net
		if (NULL == opts->out_file) {

			printf ("Setting default output format.\n");
			
			// malloc some memory for the var
			opts->out_format = (char*)malloc (5);
			
			// set png as the output format
			strncpy (opts->out_format, ".png", 5);
	
		} else if (available_formats (strrchr (opts->out_file, '.'))) {
			
			// malloc the number of byes in the extention +1 for the format var
			opts->out_format = (char*)malloc (strlen (strrchr (opts->out_file, '.'))+1);
			
			// assign the ext to the format var
			strncpy (opts->out_format, ext (opts->out_file), strlen (strrchr (opts->out_file, '.'))+1);
			
			} else {
				
				// Write out error
				printf ("format not found.\n");
				
				// free any allocated memory and exit
				cleanup_o (opts);
				exit (0);
			}
		
	} else if (available_formats (opts->out_format)) {
		// convert the string the out_format var to include the '.' for the extention
		if (strcmp ("png",opts->out_format) == 0) strncpy (opts->out_format, ".png", 5);
		if (strcmp ("svg",opts->out_format) == 0) strncpy (opts->out_format, ".svg", 5);		
		} else {
			exit(0);
		}
	
}
