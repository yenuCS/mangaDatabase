//Database class

#include "cmpt_error.h"
#include "Manga_Record.h"
#include "Database.h"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <ncurses.h>

using namespace std;

// Default constructor
Database::Database(){ 
    mangaList = readFile("database.txt");
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK); // FOR LABELS
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // FOR INFO
    init_pair(3, COLOR_GREEN, COLOR_BLACK); // HELPFUL MESSAGES
    getmaxyx(stdscr, yMax, xMax);
    refresh();
    }

// Reads in each entry in database.txt
vector<Manga_Record> Database::readFile (string input){    
    // Reads in the file containing manga manga
    vector <Manga_Record> mangaVec = {};
    fstream inFile(input);
    // Checks if file name is valid
    while (inFile.fail()){
        // Asks user if they meant something else. 
        // Yes = rerun function with correct name
        // No = quit program
        mvaddstr(1, 1,"File does not exist. Did you mean something else? (Y)es or (N)o: ");
        refresh();
        char userResponse = tolower(getch());
        while (userResponse != 'n' && userResponse != 'y' && userResponse != 'r'){
            mvaddstr(2, 1, "Your response is invalid, please try again: ");
            refresh();
            userResponse = tolower(getch());
        }
        if (userResponse == 'n'){
    
    cmpt::error("\nThe file inputted does not exist. Please try again.");}
        erase();
        mvaddstr(1, 1, "What is the name of the file?");
        echo();
        refresh();
        char correctInput [200];
        getnstr(correctInput, 190);
        noecho();
        fstream inFile(correctInput);
    }

    // Read in each input (line) from the file
    while (true){
        string entry;
        Manga_Record manga;
        getline(inFile, entry);
        if (inFile.fail()){break;}

        // Getting title
        manga.set_name (entry.substr(0, entry.find("|")));
        entry = entry.substr(entry.find("|") + 1); // Get rid of trailing |

        // Getting Genres
        string mangaGenres = entry.substr(0, entry.find("|"));
        // Get rid of starting and trailing {}|
        mangaGenres = mangaGenres.substr(1, mangaGenres.size() - 2); 
        while (true){
            string mangaGenre;
            if (mangaGenres.find(",") != -1){ // There are remaining genres
                mangaGenre = mangaGenres.substr(0, mangaGenres.find(","));
                // Get rid of comma and space
                mangaGenres = mangaGenres.substr(mangaGenres.find(",") + 2); 
                manga.add_genre(mangaGenre);}
            else { // Last genre in the list
                manga.add_genre(mangaGenres);
                break;} 
        }
        entry = entry.substr(entry.find("|") + 1);
        
        // Getting authors
        string mangaAuthors = entry.substr(0, entry.find("|"));
        // Get rid of starting and trailing {}:
        mangaAuthors = mangaAuthors.substr(1, mangaAuthors.size() - 2); 
        while (true){
            string mangaAuthor;
            if (mangaAuthors.find(",") != -1){ // There are remaining genres
                mangaAuthor = mangaAuthors.substr(0, mangaAuthors.find(","));
                mangaAuthors = mangaAuthors.substr(mangaAuthors.find(",") + 2); 
                // Get rid of comma and space
                manga.add_author(mangaAuthor);}
            else { // Last genre in the list
                manga.add_author(mangaAuthors);
                break;} 
        }
        entry = entry.substr(entry.find("|") + 1);

        // Getting Status of releasing or completed
        if (entry.substr(0, entry.find("|")) == "releasing"){manga.set_status(true);}
        entry = entry.substr(entry.find("|") + 1);

        // Getting Release Date
        manga.set_year (stoi(entry));

        // Append all this information to mangaList as its own object
        mangaVec.push_back(manga);
    }
    inFile.close();
    return mangaVec;
} 

/* -------------------------- Searching for records ------------------------- */
// Searching by exact name
vector<Manga_Record> Database::searchByExactName (string name){
    vector<Manga_Record>searchResults = {};

    for (Manga_Record manga : mangaList){

        string lowercaseName = toLowStr(name);
        string lowercaseTitle = toLowStr(manga.getName());

        if (lowercaseName == lowercaseTitle){
            searchResults.push_back(manga);
        }
        
    }
    return searchResults;
}
// Searching by sub string name
vector<Manga_Record> Database::searchBySubName (string name){
    vector<Manga_Record>searchResults = {};
    for (Manga_Record manga : mangaList){
        string lowercaseName = toLowStr(name);
        string lowercaseTitle = toLowStr(manga.getName());

        if (lowercaseTitle.find(lowercaseName) != -1){
            searchResults.push_back(manga);
        }
    }
    return searchResults;
}
// Searching by exact year
vector<Manga_Record> Database::searchByYear (int year){
    // For each manga with matching year, append to searchResults and return at end
    vector<Manga_Record> searchResults;
    for (Manga_Record manga : mangaList){
        if (manga.getYear() == year){
            searchResults.push_back(manga);
        }
    }
    if (searchResults.size() == 0){
        notFound();
    }
    return searchResults;
}
// Searching by year range
vector<Manga_Record> Database::searchByYear (int yearStart, int yearEnd){
    // For each manga within year range, append to searchResults and return at end
    vector<Manga_Record> searchResults;
    for (Manga_Record manga: mangaList){
        // Checks if the years of each manga is within the range given
        if (manga.getYear() >= yearStart && manga.getYear() <= yearEnd){
            searchResults.push_back(manga);
        }
    }
    if (searchResults.size() == 0){
        notFound();
    }
    return searchResults;
}
// Searching by author
vector<Manga_Record> Database::searchByAuthor(string name){
    vector<Manga_Record> searchResults = {};

    // Iterate through each manga entry within the vector
    for (Manga_Record manga : mangaList){
        for (string author : manga.getAuthors()){

            // Turns everything lowercase to prevent case sensitivity
            string lowercaseName = toLowStr(name);
            string lowercaseAuthor = toLowStr(author);

            // After both user input and manga's authors are made lowercase
            // .find() will check if the user's input matches any of the authors
            // Works for both exact string and substring
            if (lowercaseAuthor.find(lowercaseName) != -1){
                // If exact string or substring is found, append to searchResults
                searchResults.push_back(manga);
            }
        }
    }
    return searchResults;
}
// Searching by genres
vector<Manga_Record> Database::searchByGenre(string name){
    vector<Manga_Record> searchResults = {};

    // Iterate throughe ach manga within the vector
    for (Manga_Record manga : mangaList){
        for (string genre : manga.getGenres()){
            // Turns everything lowercase to prevent case sensitivity
            string lowercaseName = toLowStr(name);
            string lowercaseGenre = toLowStr(genre);

            // After both user input and manga's authors are made lowercase
            // .find() will check if the user's input matches any of the authors
            // Works for both exact string and substring
            if (lowercaseGenre.find(lowercaseName) != -1){
                // If exact string or substring is found, append to search results
                searchResults.push_back(manga);
            }
        }
    }
    return searchResults;
}

// Searching by status
vector<Manga_Record> Database::searchByStatus(bool status){
    vector<Manga_Record> searchResults = {};
    // Iterates through each manga entry
    for (Manga_Record manga : mangaList){
        if (status == manga.getStatus()){
            // True = releasing, false = completed.
            // Depending on bool value of status, push back entries to searchResults
            searchResults.push_back(manga);
        }
    }
    return searchResults;
}


/* ------------------- Searching for and deleting records ------------------- */
// Searching by name
void Database::deleteByExactName (string name){
    // The case where user enters in exact name of manga
    for (int i = 0; i < mangaList.size(); i++){
        string lowercaseName = toLowStr(name);
        string lowercaseTitle = toLowStr(mangaList.at(i).getName());
        if (lowercaseName == lowercaseTitle){
            bool userConfirmation = deleteConfirmation(i, mangaList.at(i));
            // Once the corresponding manga entry is found, ask user for confirmation
            // Return bool value to userConfirmation
            if (userConfirmation == true){
                // Uses the corresponding index to delete the manga entry
                mangaList.erase(mangaList.begin() + i);
                char stop = 'z';
                while (stop != 'x'){
                    erase();
                    mvaddstr(1,1,"Entry Deleted. Press \'x\' to continue.");
                    refresh();
                    stop = getch();
                }
                erase();
                
        
                return;
            }
            else {
                char stop = 'z';
                while (stop != 'x'){
                    erase();
                    mvaddstr(1,1,"Entry NOT Deleted. Press \'x\' to continue.");
                    refresh();
                    stop = getch();
                }
                erase();
                
                return;
            }
        }
    }
    notFound();
}
// Searching by sub string name
void Database::deleteBySubName (string name){
    vector<Manga_Record> searchResults = searchBySubName(name);
    if (searchResults.size() == 0){
        notFound();
        return;
    }
    printDatabase(searchResults);
    deleteConfirmationYear(searchResults);
}
// Searching by exact year
void Database::deleteByYear (int year){
    // For each manga with matching year, append to searchResults and return at end
    vector<Manga_Record> searchResults = searchByYear(year);
    
    // Display all matched entries and ask user for which one they want to delete
    printDatabase(searchResults);
    if (searchResults.size() == 0){
        notFound();
        return;}

    deleteConfirmationYear(searchResults);
}
// Searching by year range
void Database::deleteByYear (int yearStart, int yearEnd){
    // For each manga within year range, append to searchResults and return at end
    vector<Manga_Record> searchResults = searchByYear(yearStart, yearEnd);
    if (searchResults.size() == 0){
        notFound();
        return;}
    printDatabase(searchResults);
    deleteConfirmationYear(searchResults);
}

// Ask for confirmation on deleting record
bool Database::deleteConfirmation (int index, Manga_Record manga){
    // Use the parameter manga to display manga later once we have a function to do so
    char userResponse = displaySingular(index, manga);
    if (userResponse == 'y'){return true;}
    return false;
}

void Database::deleteConfirmationYear (vector<Manga_Record> searchResults){
    mvaddstr(11, 1, "Enter the entry # of the one you want to delete: ");
    echo();
    refresh();
    char userStr [4];
    getnstr(userStr, 3);
    while (!realNum (string(userStr), searchResults.size())){
        attron(COLOR_PAIR(1));
        mvaddstr(13, 1, "The entry # inputted is invalid please try again: ");
        refresh();
        getnstr(userStr, 3);
    }
    erase();
    
    noecho();
    // Account for index display, decrement by 1 for proper index
    int userInput = stoi (userStr) - 1;
    bool userConfirmation = deleteConfirmation (userInput, searchResults.at(userInput));
    erase();
    if (userConfirmation == true){
        // Loop through the entries, find matching name of user's input then delete
        for (int i = 0; i < mangaList.size(); i++){
            if (mangaList.at(i).getName() == searchResults.at(userInput).getName()){
                mangaList.erase(mangaList.begin() + i);
            }
        }
        mvaddstr(1, 1, "Entry has been deleted.");
    }
    else {mvaddstr(1, 1, "Entry has NOT been deleted.");}
    refresh();
    char stop = 'z';
    while (stop != 'x'){
        mvaddstr(3,1,"Press \'x\' to continue.");
        refresh();
        stop = getch();
    }
}

bool Database::realNum (string userInput, int end){
    for (int pos = 0; pos < userInput.length(); pos++){
        if (!(userInput[pos] >= '0' && userInput[pos] <= '9')){
            return false;}}
    if (stoi (userInput) > end || stoi(userInput) < 1){return false;}

    return true;
}

/* -------------- Listing records in multiple different orders -------------- */

//changes all characters in a string to lower case
string Database::toLowStr (string str){
    string returnStr = "";
    for (int pos = 0; pos < str.length(); pos++){
        returnStr += tolower(str[pos]);}
    return returnStr;
}

vector<Manga_Record> Database::listAlphabetical(){

    // Using selection sort, sort the manga based on the first character of names
    int minIndex = 0;
    vector<Manga_Record> alphaList = mangaList;

    // Iteration for beginning index in unsorted section of alphabeticalList
    for (int i = 0; i < alphaList.size() - 1; i++){
        // Finding the minimum element in the current unsorted alphabeticalList
        // Set index of the 'minimum element' default 
        //to the first index of unsorted alphabeticalList
        minIndex = i;
        // Iterate through the unsorted section of the vector
        for (int j = i + 1; j < alphaList.size(); j++){
            // Compare the names of the titles
            if (toLowStr(alphaList.at(j).getName()) 
                < toLowStr(alphaList.at(minIndex).getName())){
                minIndex = j;
            }
        }
        // Swap element at the minIndex with beginning of unsorted vector
        swap(alphaList.at(minIndex), alphaList.at(i));
    }
    return alphaList;
}

vector<Manga_Record> Database::listAlphabeticalReverse(){
    // Using selection sort, sort the manga based on the first character of names
    vector<Manga_Record> alphabeticalListReverse = listAlphabetical();
    reverse(alphabeticalListReverse.begin(), alphabeticalListReverse.end());
    return alphabeticalListReverse;
}

vector<Manga_Record> Database::listNumerical(){
    vector <Manga_Record> numericalList = mangaList;
    // Uses selection sort to sort each manga entry by release year
    for (int start = 0; start < numericalList.size()-1; start++){
        int min = start;
        for (int check = start + 1; check < numericalList.size(); check++){
            if (numericalList.at(check).getYear() < numericalList.at(min).getYear()){
                min = check;
            }
        }
        swap(numericalList.at(min), numericalList.at(start));
    }
    return numericalList;   
}

vector<Manga_Record> Database::listNumericalReverse(){
    vector <Manga_Record> numericalList = listNumerical();
    reverse(numericalList.begin(), numericalList.end());
    return numericalList;
}

vector<Manga_Record> Database::listAuthor(){
    // Using selection sort, sort the manga based on the first author name (alphabetical order)
    int minIndex;
    vector<Manga_Record> authorList = {};
    for (Manga_Record entry: mangaList){
        Manga_Record rec = entry;
        vector <string> alphaAuthors = {};
        for (string author: entry.getAuthors()){
            string alphaAu = toLowStr(author);
            alphaAuthors.push_back(alphaAu);
        }
        sort(begin(alphaAuthors), end(alphaAuthors));
        rec.set_authors(alphaAuthors);
        authorList.push_back(rec);
    }

    // Iteration for beginning index in unsorted section of authorList
    for (int i = 0; i < authorList.size() - 1; i++){
        // Finding the minimum element in the current unsorted authorList
        // Set index of the 'minimum element' default
        // to the first index of unsorted authorList
        minIndex = i;
        // Iterate through unsorted section of the vector
        for (int j = i + 1; j < authorList.size(); j++){
            // Compare the first authors of mangas
            if (toLowStr(authorList.at(j).getAuthors()[0])
                 < toLowStr(authorList.at(minIndex).getAuthors()[0])){
                minIndex = j;
            }
        }
        // Swap element at the minIndex with beginning of unsorted vector
        swap(authorList.at(minIndex), authorList.at(i));
    }
    return authorList;
}

vector <Manga_Record> Database::listAuthorReverse(){
    vector<Manga_Record> revAu = listAuthor();
    reverse(begin(revAu), end(revAu));
    return revAu;
}

// Getters
vector<Manga_Record> Database::getMangaList() const{ return mangaList; }; 

//Modify the vector
void Database::add_entry (Manga_Record newEntry){mangaList.push_back(newEntry);}

// Displaying a manga's information
void Database::printDatabase(vector<Manga_Record> mangaVec)
{
    char input = '0';
    int loc = 0;
    while (input != 'e')
    {
        input = displayMultiple(loc, mangaVec);
        if (input == 'c')
        {
            if (loc != mangaVec.size() - 1)
            {
                loc++;
            }
        }
        else if (input == 'z')
        {
            if (loc != 0)
            {
                loc--;
            }
        }
    }
}

char Database::displayMultiple(int index, vector<Manga_Record> mangaVec){
    erase();

    Manga_Record manga = mangaVec.at(index);
    attron(COLOR_PAIR(1));
    mvaddstr(1, 1, "Entry #");
    mvaddstr(1, 8, to_string(index + 1).data());

    mvaddstr(2, 1, "=========================================");

    attron(COLOR_PAIR(3));
    mvaddstr(3, 1, "Name: ");
    attron(COLOR_PAIR(2));
    mvaddstr(3, 7, manga.getName().data());
    mvaddstr(3, manga.getName().size() + 7, ".");

    attron(COLOR_PAIR(3));
    mvaddstr(4, 1, "Genres: ");
    attron(COLOR_PAIR(2));
    int rowPos = 9;
    for (int i = 0; i < manga.getGenres().size() - 1; i++){
        mvaddstr(4, rowPos, manga.getGenres().at(i).data());
        mvaddstr(4, rowPos + manga.getGenres().at(i).length(), ", ");
        rowPos += manga.getGenres().at(i).length() + 2;
    }
    mvaddstr(4, rowPos, manga.getGenres().at(manga.getGenres().size() - 1).data());
    mvaddstr(4, rowPos + manga.getGenres().at(manga.getGenres().size() - 1).length(), ".");
        
    
    attron(COLOR_PAIR(3));
    mvaddstr(5, 1, "Authors: ");
    attron(COLOR_PAIR(2));
    rowPos = 10;
    for (int i = 0; i < manga.getAuthors().size() - 1; i++){
        mvaddstr(5, rowPos, manga.getAuthors().at(i).data());
        mvaddstr(5, rowPos + manga.getAuthors().at(i).length(), ", ");
        rowPos += manga.getAuthors().at(i).length() + 2;
    }
    mvaddstr(5, rowPos, manga.getAuthors().at(manga.getAuthors().size()-1).data());
    mvaddstr(5, rowPos + manga.getAuthors().at(manga.getAuthors().size()-1).length(), ".");

    attron(COLOR_PAIR(3));
    mvaddstr(6, 1, "Status: ");
    attron(COLOR_PAIR(2));
    if (manga.getStatus() == true)
    {
        mvaddstr(6, 9, "Releasing.");
    }
    else
    {
        mvaddstr(6, 9, "Completed.");
    }

    attron(COLOR_PAIR(3));
    mvaddstr(7, 1, "Year of release: ");
    attron(COLOR_PAIR(2));
    mvaddstr(7, 18, to_string(manga.getYear()).data());

    attron(COLOR_PAIR(3));
    mvaddstr(9, 1, "Enter \'z\' to move backwards \'c\' to move forward and \'e\' to exit");
    refresh();
    char output = tolower(getch());
    while (output != 'z' && output != 'c' && output != 'e')
    {
        attron(COLOR_PAIR(1));
        mvaddstr(11, 1, "Enter a valid action: ");
        refresh();
        output = tolower(getch());
    }
    return output;
}

char Database::displaySingular(int index, Manga_Record manga){
// Displays information with proper formatting and colour coding

    erase();
    
    refresh();

    attron(COLOR_PAIR(1));
    mvaddstr(1,1,"You are deleting: Entry #");
    mvaddstr(1, 26, to_string(index + 1).data());


    mvaddstr(2, 1,"=========================================");
    
    attron(COLOR_PAIR(3));
    mvaddstr(3, 1, "Name: ");
    attron(COLOR_PAIR(2));
    mvaddstr(3, 7, manga.getName().data());
    mvaddstr(3, manga.getName().size() + 7, ".");

    attron(COLOR_PAIR(3));
    mvaddstr(4, 1, "Genres: ");
    attron(COLOR_PAIR(2));
    int rowPos = 9;
    for (int i = 0; i < manga.getGenres().size() - 1; i++){
        mvaddstr(4, rowPos, manga.getGenres().at(i).data());
        mvaddstr(4, rowPos + manga.getGenres().at(i).length(), ", ");
        rowPos += manga.getGenres().at(i).length() + 2;
    }
    mvaddstr(4, rowPos, manga.getGenres().at(manga.getGenres().size() - 1).data());
    mvaddstr(4, rowPos + manga.getGenres().at(manga.getGenres().size() - 1).length(), ".");
        
    
    attron(COLOR_PAIR(3));
    mvaddstr(5, 1, "Authors: ");
    attron(COLOR_PAIR(2));
    rowPos = 10;
    for (int i = 0; i < manga.getAuthors().size() - 1; i++){
        mvaddstr(5, rowPos, manga.getAuthors().at(i).data());
        mvaddstr(5, rowPos + manga.getAuthors().at(i).length(), ", ");
        rowPos += manga.getAuthors().at(i).length() + 2;
    }
    mvaddstr(5, rowPos, manga.getAuthors().at(manga.getAuthors().size()-1).data());
    mvaddstr(5, rowPos + manga.getAuthors().at(manga.getAuthors().size()-1).length(), ".");

    attron(COLOR_PAIR(3));
    mvaddstr(6, 1,"Status: ");
    attron(COLOR_PAIR(2));
    if (manga.getStatus() == true){mvaddstr(6, 9,"Releasing.");}
    else {mvaddstr(6, 9,"Completed.");}

    attron(COLOR_PAIR(3));
    mvaddstr(7, 1, "Year of release: ");
    attron(COLOR_PAIR(2));
    mvaddstr(7, 18, to_string(manga.getYear()).data());
    
    mvaddstr(9, 1, "Confirm (Y)es or (N)o: ");
    refresh();
    char userResponse = tolower(getch());
    while (userResponse != 'n' && userResponse != 'y'){
        mvaddstr(11, 1, "Your response is invalid, please try again: ");
        refresh();
        userResponse = tolower(getch());
    } 
    return userResponse;
}

void Database::notFound(){
    refresh();
    char stop = 'z';
    while (stop != 'x'){
        mvaddstr(1,1,"The manga that you are looking for cannot be found. Please try again.");
        mvaddstr(3,1, "Press \'x\' to continue");
        refresh();
        stop = getch();
    }
    erase();
}

// Deconstructor
Database::~Database(){
    endwin();}