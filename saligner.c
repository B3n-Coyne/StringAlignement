#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* saligner.c - Version 2 */

int testing_lite = 0;

int testing_full = 0;

int default_method = 1;
// 1 = global
// 0 = fitting

int max_size = 1000; // Max size for strings being aligned

/* Used in the alignment matrix to save score and */
typedef struct align_node {
    int score;
    int direction;
} Align_Node;
/* For direction:
 * 0 = / (diagonal)
 * 1 = - (left)
 * 2 = | (down)
 */

/* Function to return the cost of a match/mismatch, given two characters and the mismatch penalty */
int cost (char char1, char char2, int m) {
    if ( char1 == char2 ) {
        return 0;
    } else {
        return m * -1;
    }
}

int main (int argc, char *argv[]) {

    /* INPUT VARS */
    char *input_path;
    char *output_path;

    int global_align;

    int m; // Mismatch penalty
    int g; // Gap penalty

    FILE *input_file;
    FILE *output_file;

    /* OTHER VARS */
    char *problem_name;
    char *X_string; // Query String
    char *Y_string; // Reference String

    int x_ind; // Index in the X (query) string
    int y_ind; // Index in the Y (reference) string

    int x_len; // Length of the X (query) string
    int y_len; // Length of the Y (reference) string

    /* GET INPUTS */
    if (argc == 6) { // If inputs provided, use them
        input_path = argv[1];
        m = atoi(argv[3]);
        g = atoi(argv[4]);
        output_path = argv[5];

        /* Set alignment method */
        if (strcmp(argv[2], "fitting") == 0) {
            global_align = 0;
        } else {
            global_align = 1;
        }

    } else { // If no inputs provided, use defaults
        
        // Use default penalties
        m = 2;
        g = 5;

        // Set alignments based on default_method
        global_align = default_method;

        if (default_method == 0) {
            input_path = "default_fitting_test.txt";
            output_path = "default_saligner_fitting";
        } else {
            input_path = "default_global_test.txt";
            output_path = "default_saligner_global";
        }
    }

    /* Allocate space for the problem_name, X_string, and Y_string */
    problem_name = malloc(100 * sizeof(char));
    X_string = malloc(max_size * sizeof(char));
    Y_string = malloc(max_size * sizeof(char));

    /* Open input/output files */
    input_file = fopen(input_path, "r");
    output_file = fopen(output_path, "w");

    if (testing_lite) {
        printf("Beginning string alignment\n");
    }

    /* Read first problem header */
    fscanf(input_file, "%s\n", problem_name);

    int num_prob = 0;

    /* Create matrix */
    Align_Node ***align_matrix;

    /* Allocate space for the algnment matrix */
    align_matrix = malloc((max_size + 4) * sizeof(Align_Node**));
    for (int i = 0 ; i < (max_size + 2) ; i++) {
        align_matrix[i] = malloc((max_size + 4) * sizeof(Align_Node*));
    }

    /* Allocate cells in the alignment matrix */
    for ( x_ind = 0 ; x_ind < (max_size + 1) ; x_ind++ ) {
        for ( y_ind = 0 ; y_ind < (max_size + 1) ; y_ind++ ) {
            align_matrix[x_ind][y_ind] = malloc(sizeof(Align_Node));
            align_matrix[x_ind][y_ind] -> score = -1;
            align_matrix[x_ind][y_ind] -> direction = -1;
        }
    }

    /* PERFORM ALIGNMENT FOR EACH PROBLEM */
    while ( !feof(input_file) && problem_name != NULL) {

        /* Read the strings in */
        fscanf(input_file, "%s\n", X_string);
        fscanf(input_file, "%s\n", Y_string);

        if (testing_lite) {
            printf("Problem %d:\n", num_prob++);
            printf("\t%s\n", problem_name);
            printf("\t%s\n", X_string);
            printf("\t%s\n", Y_string);
        }

        /*********************************/
        /* PERFORM ALIGNMENT COMPUTATION */
        /*********************************/

        /* SEE SLIDE 46 IN 'Sequence similarity & alignment' COURSE SLIDES FOR PSUEDOCODE*/

        x_ind = 0;
        y_ind = 0;
        x_len = strlen(X_string);
        y_len = strlen(Y_string);

        /* Fill base cases in align_matrix */
        for ( x_ind = 0 ; x_ind < x_len + 1 ; x_ind++ ) {
            align_matrix[x_ind][0] -> score = x_ind * g * -1;
            align_matrix[x_ind][0] -> direction = 1;
        }
        for ( y_ind = 0 ; y_ind < y_len + 1 ; y_ind++ ) {
            if ( global_align ) { // Global: standard base case
                align_matrix[0][y_ind] -> score = y_ind * g * -1;
            } else { // Fitting: Allow gaps before Y
                align_matrix[0][y_ind] -> score = 0;
            }

            align_matrix[0][y_ind] -> direction = 2;
        }

        /* Loop thru and fill in align_matrix */
        for ( x_ind = 1 ; x_ind < x_len + 1 ; x_ind++ ) {
            for ( y_ind = 1 ; y_ind < y_len + 1 ; y_ind++ ) {

                /* Find costs and scores of different matchings here */
                int match_cost = cost(X_string[x_ind - 1], Y_string[y_ind - 1], m) + (align_matrix[x_ind - 1][y_ind - 1] -> score); // Cost of matching characters in X and Y
                int insert_cost = -1 * g + (align_matrix[x_ind - 1][y_ind] -> score); // Cost to insert character in Y (gap in Y)
                int delete_cost = -1 * g + (align_matrix[x_ind][y_ind - 1] -> score); // Cost to delete character in Y (gap in X)

                /* Use max score to update align_matrix[x_ind][y_ind] */
                if ( match_cost >= insert_cost && match_cost >= delete_cost ) {
                    align_matrix[x_ind][y_ind] -> score = match_cost;
                    align_matrix[x_ind][y_ind] -> direction = 0;

                } else if ( insert_cost >= delete_cost ) {
                    align_matrix[x_ind][y_ind] -> score = insert_cost;
                    align_matrix[x_ind][y_ind] -> direction = 1;

                } else {
                    align_matrix[x_ind][y_ind] -> score = delete_cost;
                    align_matrix[x_ind][y_ind] -> direction = 2;
                }

            }
        }

        /* Find starting point */
        int align_start_x = x_len;
        int align_start_y = y_len;

        if ( !global_align ) { // If fitting alignment, find the best score for where X ends

            for ( y_ind = y_len - 1 ; y_ind >= 0 ; y_ind-- ) {
                if( (align_matrix[x_len][y_ind] -> score) > (align_matrix[x_len][align_start_y] -> score) ) {
                    align_start_y = y_ind;
                }
            }

        }

        int y_start = 0;
        int y_end = align_start_y;

        // Get final score
        int final_score = align_matrix[align_start_x][align_start_y] -> score;

        if (testing_lite) {
            printf("\tscore: %d\n\ty_end: %d\n", final_score, y_end);
            // Scoring for global and fitting alignment seems to work now! Yay!
        }

        /* Backtrace thru matrix using arrows to find actual alignment */
        x_ind = align_start_x;
        y_ind = align_start_y;

        int reached_end = 0;

        char *CIGAR;
        CIGAR = malloc( 2000 * sizeof(char) );

        char *full_alignment;
        full_alignment = malloc( (max_size + 4) * sizeof(char) );
        full_alignment[0] = '\0';

        full_alignment[max_size] = '\0';
        int alignment_loc = max_size;
        // DOESN'T WORK: Use sprintf to update alignment trace

        
        while ( !reached_end ) {

            /* If arrow is diagonal, then the current characters are aligned together - check if they match */
            /* If arrow goes left, then insert into Y */
            /* If arrow goes down, then delete from Y */
            /* Move to next cell based on arrow */

            if ( align_matrix[x_ind][y_ind] -> direction == 1 ) { // Left - insert into Y
                full_alignment[--alignment_loc] = 'I';
            } else if ( align_matrix[x_ind][y_ind] -> direction == 2 ) { // Down - delete from Y
                full_alignment[--alignment_loc] = 'D';
            } else { // Diagonal - match or mismatch
                if ( X_string[x_ind - 1] == Y_string[y_ind - 1] ) {
                    full_alignment[--alignment_loc] = '=';
                } else {
                    full_alignment[--alignment_loc] = 'X';
                }
            }

            if ( global_align ) {
                if ( x_ind == 0 && y_ind == 0 ) {
                    reached_end = 1;
                }
            } else { // Fitting
                if ( x_ind == 0 ) {
                    reached_end = 1;
                    y_start = y_ind;
                }
            }

            /* Move to next cell */
            if ( align_matrix[x_ind][y_ind] -> direction == 1 ) { // Left - insert into Y
                x_ind--;
            } else if ( align_matrix[x_ind][y_ind] -> direction == 2 ) { // Down - delete from Y
                y_ind--;
            } else { // Diagonal - match or mismatch
                x_ind--;
                y_ind--;
            }
        }

        if (testing_lite) {
            printf("\ty_start: %d\n", y_start);
        }
        if (testing_full) {
            printf("\tfull_alignment: %s\n", full_alignment + alignment_loc + 1);
        }
        
        /* Convert full_alignment to CIGAR string format */
        int loc = alignment_loc + 1;
        int curr_count = 1;
        char curr_char = full_alignment[loc];

        char *buffer;
        buffer = malloc( 10 * sizeof(char) );

        CIGAR[0] = '\0';

        /* Loop thru full alignment string, counting subsiquent occurences of characters */
        while( loc < max_size ) {
            /* If character is the same as the adjacent character, incriment count */
            if ( curr_char == full_alignment[loc + 1] ) {
                curr_count++;

            /* New character, so add old count to the CIGAR string */
            } else {
                sprintf(buffer, "%d%c", curr_count, curr_char);
                strcat(CIGAR, buffer);

                curr_char = full_alignment[loc + 1];
                curr_count = 1;
            }
            loc++;
        }

        free(buffer);

        if (testing_lite) {
            printf("\tCIGAR string: %s\n", CIGAR);
        }

        /* Write info to output_file */
        fputs(problem_name, output_file);
        fputs("\n", output_file);
        fputs(X_string, output_file);
        fputs("\n", output_file);
        fputs(Y_string, output_file);
        fputs("\n", output_file);

        buffer = malloc( 200 * sizeof(char) );
        sprintf(buffer, "%d\t%d\t%d\t", final_score, y_start, y_end);

        fputs(buffer, output_file);

        fputs(CIGAR, output_file);
        fputs("\n", output_file);
        
        free(buffer);

        /*
            printf("\t%s\n", problem_name);
            printf("\t%s\n", X_string);
            printf("\t%s\n", Y_string);
        */


        free(CIGAR);
        free(full_alignment);

        /* Read next header */
        fscanf(input_file, "%s\n", problem_name);
    }

    /* Free cells in alignment matrix */
    for ( x_ind = 0 ; x_ind < (max_size + 1) ; x_ind++ ) {
        for ( y_ind = 0 ; y_ind < (max_size + 1) ; y_ind++ ) {
            free(align_matrix[x_ind][y_ind]);
        }
    }

    /* Free alignment matrix itself */
    for (int i = 0 ; i < (max_size + 2) ; i++) {
        free(align_matrix[i]);
    }
    free(align_matrix);

    /* Close files */
    fclose(input_file);
    fclose(output_file);

    /* Free other variables used */
    free(problem_name);
    free(X_string);
    free(Y_string);

    /* Get the method used */
    char *method;
    if (global_align) {
        method = "global";
    } else {
        method = "fitting";
    }

    /* Print that algorithm is done running */
    printf("Done running saligner: %s\n^-^\n", method);
    
    return 0;
}
