#include "utils.hpp"

int main() {

    cout << is_match("tpu", "tpc") << endl;
    cout << is_match("tpu", "tp+") << endl;

     cout << is_match( "upb/precis/100/*", "upb/precis/100/temperature") << endl;


    return 0;
}