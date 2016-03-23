#include <boost/test/included/unit_test.hpp>
#include "main_test.h"

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[]) 
{
    framework::master_test_suite().add(BOOST_TEST_CASE(&en_numbers_test));
    framework::master_test_suite().add(BOOST_TEST_CASE(&ru_dates_test));
    framework::master_test_suite().add(BOOST_TEST_CASE(&ru_numbers_test));
    //framework::master_test_suite().add(BOOST_TEST_CASE(&run_read_test));
    framework::master_test_suite().add(BOOST_TEST_CASE(&ru_np_test));
    framework::master_test_suite().add(BOOST_TEST_CASE(&ru_splitter_test));
    framework::master_test_suite().add(BOOST_TEST_CASE(&en_splitter_test));
    //framework::master_test_suite().add(BOOST_TEST_CASE(&dictionary_add_rem_test));
    
    return 0;
}
