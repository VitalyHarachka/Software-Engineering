#include <math.h>

#include "primeFactorisation.h"

std::list<unsigned long long int> primeFactorisation(unsigned long long int x)
{   
    //assign wheel
    long wheel[] = {1,2,2,4,2,4,2,4,6,2,6};// each number is the jump between the next prime number. 
    long  fFactor= 2; // The first possible factor.
    int tcount = 0; // spoke
    
    std::list<unsigned long long int> primeF;
    
    if(x <= 1) // Checks if the x is smaler or equal than 1, if true returns list as is.
        
    {
        return primeF;
    }
    while ( fFactor <= sqrt(x)) // check if the prime number is less than or equal to the square root of x.
    {
        if (x % fFactor == 0) // if the remander of x  equals to 0 then that means it's a prime factor and the number is put to the list and then divid it by the prime number.
        { 
            primeF.push_back(fFactor); 
            x /= fFactor; 
        } 
        else 
        {
            fFactor += wheel[tcount]; // the fFactor takes the value of the next prime factor.
            tcount = (tcount == 10) ? 3 : (tcount+1); // Insted of using an if statement I used condition '?' result_if_true (resets the 'tcount' to value 3) and ':' result_if_false (adds 1 to 'tcount'
        }
    }
    primeF.push_back(x);
    return primeF;
}


/* References
 * 
 * Programmin Praxis(2009). Wheel Factorisation. Avalable at  https://programmingpraxis.com/2009/05/08/wheel-factorization [Accessed 2 Dec. 2018].
 * 
 * En.wikipedia.org. (2018). Wheel factorization. [online] Available at: https://en.wikipedia.org/wiki/Wheel_factorization [Accessed 2 Dec. 2018].
 * 
 * En.wikipedia.org. (2018). Trial division. [online] Available at: https://en.wikipedia.org/wiki/Trial_division [Accessed 1 Dec. 2018].
 *
 * Algorithm, F. (2018). Fast Prime Factorization Algorithm. [online] Stack Overflow. Available at: https://stackoverflow.com/questions/12756335/fast-prime-factorization-algorithm/12759741?fbclid=IwAR39601ekJY1N_IonL2mOjMdzlrogK4b-BicrsikI42mndPv9x-hO4MSssk#12759742 [Accessed 2 Dec. 2018].
*/


