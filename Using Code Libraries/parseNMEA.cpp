#include "parseNMEA.h"
#include <iostream>
#include <string>



using namespace std;
namespace GPS {

bool isValidSentence(const std::string & theLog)
{
    string Sentence = theLog;
    int checksum = 0;
    
    /* Create a vector to stor the characters of the
     * string. */
    vector<char> check (Sentence.begin(), Sentence.end());
    
    // Discard the sentence exept the checksum
    Sentence.erase(Sentence.begin(), Sentence.end() - 2);
    
    /* Check if the checksum is hexadecimal. Then convert
     * the checksum to an integer.
     * Then we XOR the characters of the vector tho find 
     * the checksum.
     * And  compareit to the hex. */
    if (!isxdigit(Sentence.at(0)))
    {
        return false;
    }
    else
    {
        int hex = stol(Sentence,0,16);
        
        for (int i=1; i < check.size() - 3 ; i++)
        {
            checksum ^= check.at(i);
        }
        
        if (checksum != hex)
        {
            return false;
        }
        return true;
    }

}



NMEAPair decomposeSentence(const std::string & nmeaSentence)
{
    string sameSentence = nmeaSentence;
    vector<string> secondElement;
    size_t size;
    string token;
    
    // Remove "$" and  checksum (e.g. "*63")
    sameSentence.erase (sameSentence.begin());
    sameSentence.erase (sameSentence.find_first_of("*"),3);
    
    /* Extract the value up until the first "," and put
     * it into a vector, discard the value and repeat 
     * until the end of the string. */


    while ((size = sameSentence.find_first_of(",")) != string::npos)
    {
        token = sameSentence.substr(0,size);
        secondElement.push_back(token);
        sameSentence.erase (0,size + 1);
        
    }
    // last element of the vector
    secondElement.push_back(sameSentence);
    /* Set the first value of the vector as the first
     * element of the pair and put the other values as
     * the second element of the pair. */

    NMEAPair firstElement;
    {
        firstElement.first = secondElement[0];
        
        for (unsigned int i = 1; i <secondElement.size(); i++)
        {
            firstElement.second.push_back(secondElement[i]);
        }
    }
    
    return{firstElement.first,firstElement.second};
    
    
}
    
}


/* Only done the first two functions they pass all of the tests. Tried to 
 * do the third one but it confused me a lot, had lots of 
 * errors and didn't pass any tests and with that also couldent
 * do the 4th function because it uses all of the three previous ones.
 * hope the two first fuctions are sutable */
 



/* References:
 * 
 * Cplusplus.com. (2019). isxdigit - C++ Reference. [online] Available at: http://www.cplusplus.com/reference/cctype/isxdigit/ [Accessed 11 Feb. 2019].
 * 
 * Cplusplus.com. (2019). isxdigit - C++ Reference. [online] Available at: http://www.cplusplus.com/reference/cctype/isxdigit/ [Accessed 11 Feb. 2019].
 * 
 * [duplicate], P., Pii, V., Hasan, A. and Lindley, B. (2019). Parse (split) a string in C++ using string delimiter (standard C++). [online] Stack Overflow. Available at: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c [Accessed 13 Feb. 2019].
 * 
 * Cplusplus.com. (2019). stol - C++ Reference. [online] Available at: http://www.cplusplus.com/reference/string/stol/ [Accessed 11 Feb. 2019].
 * 
*/
