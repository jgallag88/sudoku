// Sudoku solver
/*
TODO: check for only place where a number could be in row/col/square, instead of just only value that could be in cell
      Might have to check last square to make sure we have valid solution when there are no open square remaining?
 */
#include <stdio.h>

#define SIZE 9
#define ROOT_SIZE 3

typedef struct {
    int value;
    int possVals[SIZE];
    int possibilities;
} cell;

int move(cell*);
int chooseMove(cell* board, int* row, int* col);
void printBoard(cell*);
int applyMove(cell* board, int* adjustments, int row, int col, int value);
int undoMove(cell* board, int* adjustments, int row, int col, int value);
int solve(cell*);
void analyze(cell*);
void findPossibilities(cell* board, int row, int col);

int main(void) {
    cell board[SIZE*SIZE];
    /*
    int startVals[81] = {0, 0, 3, 0, 2, 0, 6, 0, 0,
			 9, 0, 0, 3, 0, 5, 0, 0, 1,
			 0, 0, 1, 8, 0, 6, 4, 0, 0,
			 0, 0, 8, 1, 0, 2, 9, 0, 0,
			 7, 0, 0, 0, 0, 0, 0, 0, 8,
			 0, 0, 6, 7, 0, 8, 2, 0, 0,
			 0, 0, 2, 6, 0, 9, 5, 0, 0,
			 8, 0, 0, 2, 0, 3, 0, 0, 9,
			 0, 0, 5, 0, 1, 0, 3, 0, 0};

    for(int i=0; i<SIZE*SIZE; i++) {
	board[i].value = startVals[i];
    }
    */

    FILE* sudokuFile = fopen("sudoku.txt", "r");
    if(!sudokuFile) {
	printf("File not found. Exiting.");
	return -1;
    }
    
    int c;
    int cellIndex = 0;
    int puzzle = 1;
    int ignoreLine = 0;
    int pEulerSoln = 0;
    while((c = fgetc(sudokuFile)) != EOF) {
	if(ignoreLine) {
	    if(c == '\n')
		ignoreLine = 0;  // Reached end of text line, stop ignoring chars
	}
	else if(c>='0' && c<='9') {
	    // Char is an ascii digits
	    c -= '0';  // Translate from ascii char to binary value
	    board[cellIndex++].value = c;
	    
	    if(cellIndex == SIZE*SIZE) {
		// We have read in a complete puzzle
		printf("\nInput:\n\n");
		printBoard(board);
		
		if(!solve(board))
		    printf("Puzzle %i: Solved\n", puzzle);
		else
		    printf("Puzzle %i: No Solution Found\n", puzzle);
		
		printf("\nOutput:\n\n");
		printBoard(board);
		
		pEulerSoln += board[0].value*100 + board[1].value*10 + board[2].value;
		
		cellIndex = 0;  // Reset index to first
		puzzle++;
	    }
	}
	else if(c != '\n') {
	    // char is not a digit or linefeed, so we need to ignore rest of line
	    ignoreLine = 1;
	}
    }
    
    fclose(sudokuFile);
    return 0;
}

int solve(cell* board) {
    analyze(board);  // Initial analysis of board
    return move(board);  // Start backtracking algorithm
}

void analyze(cell* board) {    
    // Initialize possible values for each cell
    for(int row=0; row<SIZE; row++) {
	for(int col=0; col<SIZE; col++) {
	    for(int i=0; i<SIZE; i++) {
		board[row*SIZE + col].possVals[i] = i+1;
	    }
	}
    }

    // Remove impossible values (numbers which are already used 
    // elsewhere in row/column/square
    for(int row=0; row<SIZE; row++) {
	for(int col=0; col<SIZE; col++) {
	    if(!board[row*SIZE+col].value) {
		findPossibilities(board, row, col);
	    }
	}
    }
}

void findPossibilities(cell* board, int row, int col) {
    // Eliminate values already used in this row
    for(int j=0; j<SIZE; j++) {
	int value = board[row*SIZE + j].value;
	if(value) 
	    board[row*SIZE + col].possVals[value-1] = 0;
    }

    // Eliminate values already used in this column
    for(int i=0; i<SIZE; i++) {
	int value = board[i*SIZE + col].value;
	if(value)
	    board[row*SIZE + col].possVals[value-1] = 0;
    }

    // Eliminate values already used in this square
    int squareRow = (row/ROOT_SIZE)*ROOT_SIZE;  // Top row of this sub-square
    int squareCol = (col/ROOT_SIZE)*ROOT_SIZE;  // Left col of this sub-square
    
    for(int i=squareRow; i<squareRow+ROOT_SIZE; i++) {
	for(int j=squareCol; j<squareCol+ROOT_SIZE; j++) {
	    int value = board[i*SIZE + j].value;
	    if(value)
		board[row*SIZE + col].possVals[value-1] = 0;
	}
    }
}

// Backtracking algorithm implementation
int move(cell* board) {
    int row = 0;
    int col = 0;

    int chosenCell = chooseMove(board, &row, &col);
    if(chosenCell == -1)
	return -1;  // Failed. Were open cells, but no possible values for them
    if(chosenCell == 0)
	return 0;  // Success. No open cells left.

    // Array to track changes due to this move. Store index (row*SIZE+col) of cell for which
    // possVals was adjusted, for potentially all cells in row, col, and square (3*SIZE)
    int adjustments[3*SIZE+1];

    // Iterate over, and make move for, each possible values for this cell
    for(int i=0; i<SIZE; i++) {
	if(board[row*SIZE + col].possVals[i]) {
	    applyMove(board, adjustments, row, col, i+1);
	    if(!move(board))
		return 0; // Success
	    else
		undoMove(board, adjustments, row, col, i+1); // Undo, so we can try next value
	}
    }

    return -1;  //Failed. None of possible values for this cell were sucessful.
}

int chooseMove(cell* board, int* row, int* col) {
    *row = -1; // Row of open square with fewest possibilites
    *col = -1; // Column of open square with fewest possibilities
    int minPoss = SIZE+1;  // Number of possible values for cell with fewest

    // Find cell with fewest possible values
    for(int i=0; i<SIZE; i++) {
	for(int j=0; j<SIZE; j++) {
	    cell* c = board + i*SIZE + j;

	    // If cell does not yet have a value
	    if(!c->value) {
		// Count possible values for cell
		int possibilities = 0;
	        for(int k=0; k<SIZE; k++) {
		    if(c->possVals[k])
			possibilities++;
		}
		
		// Check for open square without possible value
		if(!possibilities) {
		    return -1; // This is a failed branch. Need to backtrack.
		}
		
		if(possibilities < minPoss) {
		    *row = i;
		    *col = j;
		    minPoss = possibilities;
		}
	    }
	}
    }

    if(*row == -1)
	return 0;  // Success. No open squares remaining
    else
	return 1; // Use board[row, col] for next move
}

int applyMove(cell* board, int* adjustments, int row, int col, int value) {
    // Apply value to cell
    board[row*SIZE + col].value = value;

    int changeNumber = 0;

    // Update possibilites for remaining cells
    // In row...
    for(int j=0; j<SIZE; j++) {
	//int value = board[row*SIZE + j].value;
	if(!board[row*SIZE + j].value) {
	    int* possibility = &(board[row*SIZE + j].possVals[value-1]);
	    if(*possibility) {
		*possibility = 0;  // This is no longer a possibility
		adjustments[changeNumber++] = row*SIZE+j; // Add it to list of adjustmemts
	    }
	}
    }

    // ... in this column ...
    for(int i=0; i<SIZE; i++) {
	//int value = board[i*SIZE + col].value;
	if(!board[i*SIZE + col].value) {
	    int* possibility = &(board[i*SIZE + col].possVals[value-1]);
	    if(*possibility) {
		*possibility = 0;
		adjustments[changeNumber++] = i*SIZE+col;
	    }
	}
    }

    // ... and in this square.
    int squareRow = (row/ROOT_SIZE)*ROOT_SIZE;  // Top row of this sub-square
    int squareCol = (col/ROOT_SIZE)*ROOT_SIZE;  // Left col of this sub-square
    
    for(int i=squareRow; i<squareRow+ROOT_SIZE; i++) {
	for(int j=squareCol; j<squareCol+ROOT_SIZE; j++) {
	    //int value = board[i*SIZE + j].value;
	    if(!board[i*SIZE + j].value) {
		int* possibility = &(board[i*SIZE + j].possVals[value-1]);
		if(*possibility) {
		    *possibility = 0;
		    adjustments[changeNumber++] = i*SIZE+j;
		}
	    }
	}
    }

    adjustments[changeNumber] = -1;  // Signal end of adjustments
    return 0;
}

int undoMove(cell* board, int* adjustments, int row, int col, int value) {
    board[row*SIZE + col].value = 0;

    // Undo adjustments
    int changeNumber = 0;
    while(adjustments[changeNumber] != -1) {
	board[adjustments[changeNumber++]].possVals[value-1] = value;
    }

    return 0;
}

void printBoard(cell* board) {
    char boardFormat[] = "%i | %i | %i || %i | %i | %i || %i | %i | %i\n";

    for(int i=0; i<SIZE; i++) {
	printf(boardFormat,
	       board[i*SIZE].value,
	       board[i*SIZE + 1].value,
	       board[i*SIZE + 2].value,
	       board[i*SIZE + 3].value,
	       board[i*SIZE + 4].value,
	       board[i*SIZE + 5].value,
	       board[i*SIZE + 6].value,
	       board[i*SIZE + 7].value,
	       board[i*SIZE + 8].value);
	if(i==2 || i==5)
	    printf("===================================\n");
	else if(i != 8)
	    printf("--+---+---++---+---+---++---+---+--\n");
    }
}
