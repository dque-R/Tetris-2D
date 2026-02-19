#include <ncurses.h> // graphic design
#include <stdlib.h> // for data(rand, srand)
#include <string.h> // for strlen(), strcpy()
#include <stdio.h> // for files 
#include <unistd.h> // for usleep
#include <time.h> // for working with time, difftime
#include <stdbool.h> 
// gcc program.c -o program -lncurses
// cd /mnt/c/Users/Dell/Desktop/vscode/ps6

// sizes and constants 
#define WIDTH_OF_THE_FIELD 12 // width of the gaming field in blocks
#define HEIGHT_OF_THE_FIELD 22 // height of the gaming field in blocks 
#define WIDTH_INFO 17 // width of the info panel
#define WIDTH_OF_THE_NEXT 12 // width of the panel "Next"
#define WIDTH_OF_CONTROLS 20 // width of the control panel

// width of the playing board in cells (12x22)
#define BOARD_WIDTH 12 // width of the board 
#define BOARD_HEIGHT 22 // height of the board
#define SCORING_FILE "Tetris_scores.txt" // for saving the records

//structures for figures

typedef struct {
    int x, y; // coord of centre of the figures on the field 
    int figures[4][2]; // coords of 4 blocks of the figure depends on the centre
    int color; // color of the figure
} Piece;


//GLOBAL VARIABLES

int field[BOARD_HEIGHT][BOARD_WIDTH]; // PLAYING FIELD  
Piece current, next_piece; // current and the next figure
int score = 0; // score of the player
int level = 1; // current level of the game
int lines = 0; // quatity of the broken lines 
int game_is_over = 0; //(if 1, game is over)
int game_is_paused = 0; // if 1, game is paused
int selected_level = 1;
int speed_of_drop = 500; // drop speed (ms)
char name_of_the_player[50] = "Player";
time_t last_drop_time; // time of the last drop

//Definition of 7 types of tetromino
//Each type is 4 blocks with coordinates relative to the center of the figure
//The first number is the x coordinate, the second is the y coordinate
int figures[7][4][2] = {
    {{0,1}, {1,1}, {2,1}, {3,1}}, // figure "I" (long stick)
    {{0,0}, {0,1}, {1,1}, {2,1}}, // figure J
    {{2,0}, {0,1}, {1,1}, {2,1}}, // figure L
    {{1,0}, {2,0}, {1,1}, {2,1}}, // square
    {{1,0}, {2,0}, {0,1}, {1,1}}, // figure S
    {{1,0}, {0,1}, {1,1}, {2,1}}, // figure T
    {{0,0}, {1,0}, {1,1}, {2,1}} // figure Z

};



void all_colors(){
    start_color();

    use_default_colors(); // default colors of terminal 
    
    init_pair(1, COLOR_BLUE, COLOR_BLACK); // FIELD
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // FRAME 
    init_pair(3, COLOR_CYAN, COLOR_BLACK); // CAPTIONS
    init_pair(4, COLOR_GREEN, COLOR_BLACK); // DEFINITIONS
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); // LINES
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // CONTROL

    //COLOR OF THE FIGURES
    init_pair(7, COLOR_BLACK, COLOR_CYAN); // I
    init_pair(8, COLOR_BLACK, COLOR_BLUE); // J
    init_pair(9, COLOR_BLACK, COLOR_YELLOW); // L
    init_pair(10, COLOR_BLACK, COLOR_GREEN); // square
    init_pair(11, COLOR_BLACK, COLOR_MAGENTA); // S
    init_pair(12, COLOR_BLACK, COLOR_RED); // T
    init_pair(13, COLOR_BLACK, COLOR_WHITE);
}

//Function to center text
// y - line on screen
// starting_x - starting x-coordinate
// width - width of area to center
// text - text to display
void centered_text(int y, int starting_x, int width, const char *text){

    int length = strlen(text); // length of the text
    int x = starting_x + (width - length) / 2; // for centre 
    mvprintw(y, x, "%s", text); // printing text on the screen
}

// DRAWING FRAME 
// Draw a frame with given parameters
// y, x - coordinates of the upper left corner
// height, width - height and width of the frame

void paint_frame(int y, int x, int height, int width){ 
    attron(COLOR_PAIR(2));

    //ANGLES
    mvaddch(y, x, '+');
    mvaddch(y, x + width - 1, '+');
    mvaddch(y + height - 1, x, '+');
    mvaddch(y + height - 1, x + width - 1, '+');

    // vertical lines
    for( int i = 1; i < height - 1; i++){
        mvaddch(y + i, x, '|');
        mvaddch(y + i, x + width - 1, '|');
    }
    //horizontal lines
    for( int j = 1; j < width - 1; j++){
        mvaddch(y, x + j, '-');
        mvaddch(y + height - 1, x + j, '-');
    }

    //fulfilling with spaces 
    for ( int i = 1; i < height - 1; i++){
        for ( int j = 1; j < width - 1; j++){
            mvaddch(y + i, x + j, ' ');
        }
    }
    attroff(COLOR_PAIR(2));
}

// Drawing the playing field (empty)
// starting_y, starting_x - coordinates of the upper left corner of the field
void paint_field(int starting_y, int starting_x){
    
    int field_width = WIDTH_OF_THE_FIELD * 2; // 2 SYMBOLS PER CELL

    //DRAWING FRAME OF THE FIELD 
     paint_frame(starting_y, starting_x, HEIGHT_OF_THE_FIELD + 2, field_width + 2);

    //fulfilling with spaces(empty field)
    attron(COLOR_PAIR(1));
    for(int y = 0; y < HEIGHT_OF_THE_FIELD; y++){
        for(int x = 0; x < field_width; x++){
            mvaddch(starting_y + 1 + y, starting_x + 1 + x, ' ');
        }
    }
    attroff(COLOR_PAIR(1));
}

// Drawing the information panel (score, level, lines)
// starting_y, starting_x - coordinates of the upper left corner of the panel
void paint_info_panel(int starting_y, int starting_x){
    int height_info = 15;
    paint_frame(starting_y, starting_x, height_info, WIDTH_INFO + 2);

    //CAPTION "TETRIS"
    attron(COLOR_PAIR(3) | A_BOLD); // Enable title color + bold text
    centered_text(starting_y + 1, starting_x, WIDTH_INFO + 2, "TETRIS");

    //SCORE
    attron(COLOR_PAIR(3));
    centered_text(starting_y + 3, starting_x, WIDTH_INFO + 2, "Score");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(4) | A_BOLD);
    centered_text(starting_y + 4, starting_x, WIDTH_INFO + 2, "0"); // Space for the values
    attroff(COLOR_PAIR(4) | A_BOLD);

    //level
    attron(COLOR_PAIR(3));
    centered_text(starting_y + 6, starting_x, WIDTH_INFO + 2, "Level");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(4) | A_BOLD);
    centered_text(starting_y + 7, starting_x, WIDTH_INFO + 2, "0"); // Space for the values
    attroff(COLOR_PAIR(4) | A_BOLD);

    //LINES
    attron(COLOR_PAIR(3));
    centered_text(starting_y + 9, starting_x, WIDTH_INFO + 2, "Lines");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(5) | A_BOLD);
    centered_text(starting_y + 10, starting_x, WIDTH_INFO + 2, "10"); // Space for the values
    attroff(COLOR_PAIR(5) | A_BOLD);
}

// Draw the panel for the next shape
// starting_y, starting_x - coordinates of the upper left corner of the panel
void paint_next_panel(int starting_y, int starting_x){

    int height_of_the_panel = 8;
    paint_frame(starting_y, starting_x, height_of_the_panel, WIDTH_OF_THE_NEXT + 2);

    //CAPTION "NEXT"
    attron(COLOR_PAIR(3) | A_BOLD);
    centered_text(starting_y + 1, starting_x, WIDTH_OF_THE_NEXT + 2, "NEXT");
    attroff(COLOR_PAIR(3) | A_BOLD);

    //empty space for the next symbol
    attron(COLOR_PAIR(1));
    for(int y = 0; y < 4; y++){
        for(int x = 0; x < WIDTH_OF_THE_NEXT; x++){
            mvaddch(starting_y + 3 + y, starting_x + 1 + x, ' ');
        }
    }
    attroff(COLOR_PAIR(1));
}

// Drawing the control panel (key list)
// starting_y, starting_x - coordinates of the upper left corner of the panel
void draw_control_panel(int starting_y, int starting_x){
    int height_of_the_panel = 11;
    paint_frame(starting_y, starting_x, height_of_the_panel, WIDTH_OF_CONTROLS + 2);

    //caption "CONTROLS"
    attron(COLOR_PAIR(3) | A_BOLD);
    centered_text(starting_y + 1, starting_x, WIDTH_OF_CONTROLS + 2, "CONTROLS");
    attroff(COLOR_PAIR(3) | A_BOLD);

    //info about how to play
    attron(COLOR_PAIR(6));
    mvprintw(starting_y + 3, starting_x + 2, "Left/Right: Move");
    mvprintw(starting_y + 4, starting_x + 2, "Up        : Rotate");
    mvprintw(starting_y + 5, starting_x + 2, "Down      : Speed");
    mvprintw(starting_y + 6, starting_x + 2, "Space     : Drop");
    mvprintw(starting_y + 7, starting_x + 2, "P         : Pause");
    attroff(COLOR_PAIR(6));
}

// Gameboard initialization
// Fills the entire board with zeros (empty cells)
void board_initialization(){
    for(int y = 0; y < BOARD_HEIGHT; y++){
        for(int x = 0; x < BOARD_WIDTH; x++){
            field[y][x] = 0; // 0 means empty cell
        }
    }
}

// Creating a new shape
// type - shape type (0-6)
// p - pointer to the Piece structure to be filled
void create_figure(Piece *p, int type){
    p->color = type + 7; // Color = type + 7 (because the first 6 pairs are for the interface)
    p->x = 4; // Initial X-coordinate (center of the field horizontally)
    p->y = 0; // Initial Y coordinate (top of field)
    
    for(int i = 0; i < 4; i++){
        p->figures[i][0] = figures[type][i][0];
        p->figures[i][1] = figures[type][i][1];
    }
}

// Check for collision of the shape with other objects
// p - pointer to the shape
// dx, dy - offset to check
// Returns 1 if there is a collision, 0 if not
int check_of_the_collision(Piece *p, int dx, int dy){
    for(int i = 0; i < 4; i++){
        // Calculate the coordinates of each block of the shape after displacement
        int x = p->x + p->figures[i][0] + dx;
        int y = p->y + p->figures[i][1] + dy;

        // Check for horizontal field overflow
        if(x < 0 || x >= BOARD_WIDTH){
            return 1; // Collision with the wall
        }

        // Check for vertical out-of-bounds
        if(y > BOARD_HEIGHT){
            return 1; // Collision with bottom
        }

        // Check for overlap with already fixed shapes
        if(y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH && field[y][x] != 0){
            return 1; // Collision with another shape
        }
        
        if(y == BOARD_HEIGHT){
            return 1;
        }
    }
return 0;
}

// Fixing the figure on the playing field (transforming it into a part of the field)
// p - pointer to the figure
void fix_figure(Piece *p){
    for(int i = 0; i < 4; i++){
        int x = p->x + p->figures[i][0];
        int y = p->y + p->figures[i][1];

        if(y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH){
            field[y][x] = p->color;
        }
    }
}

// Rotate the shape 90 degrees clockwise
// p - pointer to the shape
void rotate_figure(Piece *p){
    Piece temp = *p; // temporary copy of the figure 

    // Formula for rotating a matrix 90 degrees: (x, y) -> (-y, x)
    for(int i = 0; i < 4; i++){
        int x = p->figures[i][0];
        
        int y = p->figures[i][1];
        temp.figures[i][0] = -y;
        temp.figures[i][1] = x;
    }

    // If there are no collisions after rotation - apply rotation
    if(!check_of_the_collision(&temp, 0, 0)){
        *p = temp;
    }
}

// Delete filled lines and return the number of deleted lines
int clearing_the_lines(){
    int alr_cleared = 0;

    // checking lines from bottom to up
    for(int y = BOARD_HEIGHT - 1; y >= 0; y--){
        int full = 1;
        // Check if the entire line is filled
        for(int x = 0; x < BOARD_WIDTH; x++){
            if(field[y][x] == 0){
                full = 0;
                break;
            }
        }

        // If the line is completely filled
        if(full){
            alr_cleared++; // Increment the counter

            // Shift all lines above down one position
            for(int yy = y; yy > 0; yy--){
                for(int x = 0; x < BOARD_WIDTH; x++){
                    field[yy][x] = field[yy-1][x];
                }
            }
            
            // Clear the top line (after shifting)
            for(int x = 0; x < BOARD_WIDTH; x++){
                field[0][x] = 0;
            }
            y++; // Recheck this same position (because the lines have shifted)
        }
    }
    return alr_cleared;
}

// Update game statistics after lines are cleared
// alr_cleared - number of lines cleared simultaneously
void updating_stats(int alr_cleared){
    if(alr_cleared > 0){
        lines += alr_cleared; // Add to the total number of lines

        // Points are awarded depending on the number of lines removed simultaneously
        switch(alr_cleared){
            case 1: score += 100 * level; break;
            case 2: score += 300 * level; break;
            case 3: score += 500 * level; break;
            case 4: score += 800 * level; break;
        }
        
        // Automatic level increase every 5 lines destroyed
        int new_level = selected_level + (lines / 5);
        if(new_level > 3){
            new_level = 3;
        }
        // Якщо рівень змінився - оновлюємо швидкість
        if(new_level != level){
            level = new_level;
            if(level == 1) speed_of_drop = 800;
            else if(level == 2) speed_of_drop = 500;
            else speed_of_drop = 300;
        }
    }
}

// Rendering the entire playing field (fixed figures)
// field_y, field_x - coordinates of the upper left corner of the field on the screen
void paint_game_field(int field_y, int field_x){
    for(int y = 0; y < BOARD_HEIGHT; y++){
        for(int x = 0; x < BOARD_WIDTH; x++){

            // Calculate screen coordinates for each cell
            int screen_y = field_y + y + 1;
            int screen_x = field_x + 1 + (x * 2);

            if(field[y][x] > 0){
                attron(COLOR_PAIR(field[y][x]));
                mvaddch(screen_y, screen_x, '[');
                mvaddch(screen_y, screen_x + 1, ']');
                attroff(COLOR_PAIR(field[y][x]));
            }
        }
    }
}

// Rendering the current falling shape
// field_y, field_x - coordinates of the upper left corner of the field on the screen
void paint_current_figure(int field_y, int field_x){
    attron(COLOR_PAIR(current.color));

    for(int i = 0; i < 4; i++){
        int x = current.x + current.figures[i][0];
        int y = current.y + current.figures[i][1];
        
        // Draw only visible blocks (those not above the top edge)
        if(y >= 0){ 
            int screen_y = field_y + y + 1;
            int screen_x = field_x + 1 + (x * 2);

            mvaddch(screen_y, screen_x, '[');
            mvaddch(screen_y, screen_x + 1, ']');
        }
    }
    attroff(COLOR_PAIR(current.color));
}

// Draw the next shape in a separate panel
// panel_y, panel_x - coordinates of the "Next" panel
void paint_next_figure_panel(int panel_y, int panel_x){
    attron(COLOR_PAIR(next_piece.color));

    for(int i = 0; i < 4; i++){
        int x = next_piece.figures[i][0];
        int y = next_piece.figures[i][1];

        // Обчислення екранних координат в панелі "Next"
        int screen_y = panel_y + 3 + y; // +3 to center the figure on the panel
        int screen_x = panel_x + 4 + (x * 2); // +4 for horizontal centering

        mvaddch(screen_y, screen_x, '[');
        mvaddch(screen_y, screen_x + 1, ']');

    }
     attroff(COLOR_PAIR(next_piece.color));
}

// Start a new game (reset all parameters)
void new_game_starting(){
    board_initialization();
    score = 0;
    lines = 0;
    level = selected_level;

    // Setting the fall speed depending on the level
    if(selected_level == 1) speed_of_drop = 800;
        else if(selected_level == 2) speed_of_drop = 500;
        else speed_of_drop = 300;

        game_is_over = 0; // game is not finished
        game_is_paused = 0; // game is not paused

        // Create the current and next shapes
        create_figure(&current, rand() % 7);
        create_figure(&next_piece, rand() % 7);

        last_drop_time = time(NULL); // Remember the time of the last fall
}

// Save the current game to a file
void save_stats(){
    FILE *f = fopen("Tetris_save.txt", "w"); // Open the file for writing
    if(f){
        fprintf(f, "%d %d %d %d %d\n", score, lines, level, selected_level, game_is_over); // Save the main game parameters

        // Save the state of the playing field
        for(int y = 0; y < BOARD_HEIGHT; y++){
            for(int x = 0; x < BOARD_WIDTH; x++){
                fprintf(f, "%d ", field[y][x]);
            }
            fprintf(f, "\n");
        }
        fclose(f);

        // Message about successful saving
        mvprintw(0, 0, "Game saved!          ");
        refresh();
        usleep(800000); // delay 800 ms 
    }
}

// loading game from the file 
void load_game_stats(){
     FILE *f = fopen("Tetris_save.txt", "r"); // open file for reading 
    if(f){
        //loading main game options
        fscanf(f, "%d %d %d %d %d\n", &score, &lines, &level, &selected_level, &game_is_over);

        //loading state of the playing field
         for(int y = 0; y < BOARD_HEIGHT; y++){
            for(int x = 0; x < BOARD_WIDTH; x++){
                fscanf(f, "%d ", &field[y][x]);
            }
        }
        fclose(f);

        // Message about successful loading
        mvprintw(0, 0, "Game loaded!          ");
        refresh();
        usleep(800000);
    }else{
        mvprintw(0, 0, "No save file found     ");
        refresh();
        usleep(800000);
    }
    
}

// Difficulty level selection menu
// max_y, max_x - terminal dimensions
void select_level_menu(int max_y, int max_x){
    clear();

    //caption menu
    attron(COLOR_PAIR(3) | A_BOLD);
    centered_text(max_y / 2 - 3, 0, max_x, "SELECT LEVEL");
    attroff(COLOR_PAIR(3) | A_BOLD);

    // variants of levels
    attron(COLOR_PAIR(4));
    centered_text(max_y / 2 - 1, 0, max_x, "1 - Easy (slow)");
    centered_text(max_y / 2, 0, max_x, "2 - Medium");
    centered_text(max_y / 2 + 1, 0, max_x, "3 - Hard (fast)");
    attroff(COLOR_PAIR(4));

    // Instruction
    attron(COLOR_PAIR(5));
    centered_text(max_y / 2 + 3, 0, max_x, "Press 1, 2 or 3");
    attroff(COLOR_PAIR(5));

    refresh();

    // WAITING FOR USER INPUT
    int c;
    while(true){
        c = getch(); // Wait for a key press
        if(c == '1'){
        selected_level = 1; // easy level
         break;
        }
        if(c == '2'){
        selected_level = 2; // medium level
         break;
        }
        if(c == '3'){
        selected_level = 3; // hard level
         break;
        }
        if(c == 'q' || c == 'Q'){
        endwin();
        exit(0); // quiting from the program
        }
    }
}

// Save the record to file
void save_highest_score(){
    FILE *f = fopen(SCORING_FILE, "a"); // Open the file for addition
    if(f){
        // Record: name, account, level, number of lines
        fprintf(f, "%s %d %d %d\n", name_of_the_player, score, level, lines);
        fclose(f);
    }
}



int main(){
    
    srand(time(NULL)); // Initialize the random number generator

    //initialize ncurses
    initscr();
    cbreak(); // Disable input buffering (keys are processed immediately)
    noecho(); // Do not display entered characters
    curs_set(0); // hide cursor
    keypad(stdscr, TRUE); // Enable special key processing (arrows)

    all_colors();  //initializing the colors

    //getting size if the terminal in order to make all centered
    int max_y;
    int max_x;
    getmaxyx(stdscr, max_y, max_x);

    // ENTER PLAYER NAME
    echo();
    clear();
    attron(COLOR_PAIR(3));
    centered_text(max_y / 2 - 1, 0, max_x, "Enter your name:");
    attron(COLOR_PAIR(4));
    mvprintw(max_y / 2 + 1, max_x/2 - 10, "> ");
    getnstr(name_of_the_player, 49); // Read the name (maximum 49 characters)
    noecho();

    select_level_menu(max_y, max_x);

    //calculate the total width of all elements
    int width_of_field_display = WIDTH_OF_THE_FIELD * 2 + 2;
    int total_width = width_of_field_display + WIDTH_INFO + WIDTH_OF_CONTROLS + 8;

    //horizontal centering
    int starting_x = ( max_x - total_width) / 2;
    if(starting_x < 2){
        starting_x = 2;
    }
    
    //height for gorgeous vertical centering
    int total_height = HEIGHT_OF_THE_FIELD + 6;
    int starting_y = (max_y - total_height) / 2;
    if(starting_y < 3){
        starting_y = 3;
    }

    //positions for each element(all are centered)
    int y_field = starting_y; // Y field
    int x_field = starting_x; // X field

    int y_info = starting_y; // Y info panel
    int x_info = x_field + width_of_field_display + 3; // X infopanel

    int y_next = starting_y + 16; // Y "next" panel
    int x_next = x_field + width_of_field_display + 3; // X "next" panel

    int y_controls = y_next + 9; // Y controls panel
    int x_controls = x_field + width_of_field_display + 3; // X controls panel

    new_game_starting();

    int running = 1; // Main loop operation flag
    timeout(100); // Timeout for getch() - 100 ms

    while(running){
    erase(); // clearing the screen

    //caption of the game 
    attron(COLOR_PAIR(3) | A_BOLD);
    centered_text(starting_y - 2, 0, max_x, "TETRIS");
    attroff(COLOR_PAIR(3) | A_BOLD);

    //drawing all elements

    paint_field(y_field, x_field);
    paint_info_panel(y_info, x_info);
    paint_next_panel(y_next, x_next);
    draw_control_panel(y_controls, x_controls);
    
    // GAME DRAWING
    paint_game_field(y_field, x_field); // fixed figure
    paint_current_figure(y_field, x_field); // current figure
    paint_next_figure_panel(y_next, x_next); // next figure

    // UPDATE NUMERICAL VALUES ON THE INFOPANEL
    char score_str[10], level_str[10], lines_str[10];
    sprintf(score_str, "%d", score);
    sprintf(level_str, "%d", level);
    sprintf(lines_str, "%d", lines);

    attron(COLOR_PAIR(4) | A_BOLD);
    centered_text(y_info + 4, x_info, WIDTH_INFO + 2, score_str); // Score
    centered_text(y_info + 7, x_info, WIDTH_INFO + 2, level_str); // Level
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(5) | A_BOLD);
    centered_text(y_info + 10, x_info, WIDTH_INFO + 2, lines_str); // lines
    attroff(COLOR_PAIR(5) | A_BOLD);    

    //STATUS information
    attron(COLOR_PAIR(6));
    if(game_is_over){
    
        //Status "Game is over"
        mvprintw(max_y - 3, 2, "Status: GAME OVER! | Level: %d | Score: %d", level, score);
    }else if(game_is_paused){
        //Status "Game is paused"
        mvprintw(max_y - 3, 2, "Status: PAUSED | Level: %d | Score: %d", level, score, speed_of_drop);
    }else{
        //Status "Game is on"
        mvprintw(max_y - 3, 2, "Status: PLAYING | Level: %d | Score: %d | Speed: %d", level, score, speed_of_drop);   
    }
    
    // USER ACTIONS (line above status)
    attron(COLOR_PAIR(4));
    if(game_is_over){
        mvprintw(max_y - 2, 2, "Action: Press 'r' to restart or 'q' to quit");
    }else if(game_is_paused){
        mvprintw(max_y - 2, 2, "Action: Game paused. Press 'p' to resume");
    }else{
        mvprintw(max_y - 2, 2, "Action: Use arrow keys to move and rotate");
    }
    attroff(COLOR_PAIR(4));

    // LIST OF ALL COMMANDS (bottom line)
    attron(A_BOLD | COLOR_PAIR(5));
    mvprintw(max_y - 1, 2, "Controls: Arrows | Space: Drop | P: Pause | R: Restart | S: Save | L: Load | Q: Quit");
    attroff(A_BOLD | COLOR_PAIR(5));
    refresh();

    int ch = getch(); // Get the pressed key (with a timeout of 100 ms)

    // EXIT THE GAME
    if(ch == 'q' || ch == 'Q'){
        running = 0;
    }

    // INPUT PROCESSING DURING ACTIVE GAME (not paused, not completed)
    if(!game_is_over && !game_is_paused){
        int mooved = 0; // Motion flag (not used?)

        switch(ch){
            case KEY_LEFT:
                if(!check_of_the_collision(&current, -1, 0)){
                    current.x--;
                    mooved = 1;
                    mvprintw(max_y - 2, 2, "Action: Moving left           ");
                }
                break;
            case KEY_RIGHT:
                if(!check_of_the_collision(&current, 1, 0)){
                    current.x++;
                    mvprintw(max_y - 2, 2, "Action: Moving right           ");
                }
                break;
            case KEY_UP:
                rotate_figure(&current);
                mvprintw(max_y - 2, 2, "Action: Rotating piece          ");
                break;
            case KEY_DOWN:
                if(!check_of_the_collision(&current, 0, 1)){
                    current.y++;
                    mooved = 1;
                    score++;
                    mvprintw(max_y - 2, 2, "Action: Speeding up           ");
                }
                break;
            case ' ':
                while(!check_of_the_collision(&current, 0, 1)){
                    current.y++;
                    score += 2;
                }
                fix_figure(&current); // Fixing the shape
                updating_stats(clearing_the_lines()); // update stats
                current = next_piece; // next figure becomes current
                create_figure(&next_piece, rand() % 7); // random generation of the next new figure

                // CHECK FOR GAME END (after a quick fall)
                if(check_of_the_collision(&current, 0, 0)){
                    game_is_over = 1;
                    save_highest_score();
                    timeout(-1);
                    clear();

                    // Show the game completion message
                    attron(COLOR_PAIR(6));
                    mvprintw(max_y - 3, 2, "Status: GAME OVER! | Level: %d | Score: %d", level, score);
                    attron(COLOR_PAIR(4));
                    mvprintw(max_y - 2, 2, "Action: Press 'r' to restart or q to quit");
                    refresh();
                    napms(1000);
                    timeout(100);
                }
                mvprintw(max_y - 2, 2, "Action: Dropping piece        ");
                break;
            case 's':
            case 'S':
                save_stats();
                mvprintw(max_y - 2, 2, "Action: Game saved        ");
                break;
            case 'l':
            case 'L':
                load_game_stats();
                mvprintw(max_y - 2, 2, "Action: Game loaded       ");
                break;
            }

            // AUTOMATIC SHAPE FALLING OVER TIME
            time_t now = time(NULL);

            if(difftime(now, last_drop_time) * 1000 >= speed_of_drop){
                if(!check_of_the_collision(&current, 0, 1)){
                    current.y++;
                }else{
                    // Фіксація фігури та перевірка ліній
                    fix_figure(&current);
                    updating_stats(clearing_the_lines());
                    current = next_piece;
                    create_figure(&next_piece, rand() % 7);

                    // ПЕРЕВІРКА НА ЗАВЕРШЕННЯ ГРИ (після автоматичного падіння)
                    if(check_of_the_collision(&current, 0, 0)){
                        game_is_over = 1;
                        save_highest_score();
                    }
                }
                last_drop_time = now; // Update the last fall time
            }
    
         }

         // PROCESSING OTHER COMMANDS (always work)
         switch(ch){
            case 'p':
            case 'P':
                if(!game_is_over){
                    game_is_paused = !game_is_paused; // Pause switch
                    mvprintw(max_y - 2, 2, "Action: Game %s         ", game_is_paused ? "paused" : "resumed");
                }
                break;
            case 'r':
            case 'R':
                new_game_starting();
                mvprintw(max_y - 2, 2, "Action: Game reset          ");
                break;
         }
     }

     clear();
     attron(A_BOLD | COLOR_PAIR(3));
     centered_text(max_y / 2, 0, max_x, "Thank you for playing!");

     char messg[50];
     sprintf(messg, "Final score: %d", score);
     centered_text(max_y / 2 + 1, 0, max_x, messg);

     centered_text(max_y / 2 + 2, 0, max_x, "Press any key to exit...");
     attroff(A_BOLD | COLOR_PAIR(3));

     refresh();
     timeout(-1); // Block waiting for input
     getch(); // Wait for any key

     endwin(); // Finishing up with ncurses

    return 0;
}
