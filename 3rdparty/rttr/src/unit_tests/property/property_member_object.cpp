/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2017 Axel Menzel <info@rttr.org>                     *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#include <rttr/registration>

#include <iostream>
#include <memory>
#include <functional>

#include <catch/catch.hpp>

using namespace rttr;
using namespace std;

static int g_invalid_instance = 0;

using func_ptr = void(*)(int);
struct property_member_obj_test
{
    property_member_obj_test()
    : _p1(0), _p3(1000, 42)
    {
    }


    int                 _p1;
    const int           _p2 = 12;
    std::vector<int>    _p3;
    std::vector<int>    _p4 = std::vector<int>(50, 12);
    variant             _p7 = std::string("hello");
    const variant       _p8 = 23;
    int*                _p9 = nullptr;
    int*                _p10 = &_p1;



    RTTR_REGISTRATION_FRIEND;
};


/////////////////////////////////////////////////////////////////////////////////////////
// init static variables and some global functions added as properties to the test class



static void my_callback(int)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

RTTR_REGISTRATION
{
    registration::class_<property_member_obj_test>("property_member_obj_test")
        .property("p1",    &property_member_obj_test::_p1) ( metadata("Description", "Some Text") )
        .property_readonly("p2",    &property_member_obj_test::_p2) ( metadata("Description", "Some Text") )
        .property("p3",    &property_member_obj_test::_p3)
        (
            metadata("Description", "Some Text"),
            policy::prop::bind_as_ptr
        )
         .property_readonly("p4",    &property_member_obj_test::_p4)
        (
            metadata("Description", "Some Text"),
            policy::prop::bind_as_ptr
        )
        .property("p5",    &property_member_obj_test::_p3)
        (
            metadata("Description", "Some Text"),
            policy::prop::as_reference_wrapper
        )
         .property_readonly("p6",    &property_member_obj_test::_p4)
        (
            metadata("Description", "Some Text"),
            policy::prop::as_reference_wrapper
        )
        .property("p7", &property_member_obj_test::_p7)
        .property_readonly("p8", &property_member_obj_test::_p8)
        .property("p9", &property_member_obj_test::_p9)
        .property_readonly("p10", &property_member_obj_test::_p10)
        ;
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);
    REQUIRE(prop_type.is_valid() == true);

    property prop = prop_type.get_property("p1");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == false);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == false);
    CHECK(prop.get_type() == type::get<int>());
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    CHECK(prop.set_value(obj, 42) == true);
    CHECK(prop.get_value(obj).is_type<int>() == true);
    CHECK(prop.get_value(obj).get_value<int>() == 42);

    // invalid invoke
    CHECK(prop.set_value(obj, "test") == false);
    CHECK(prop.get_value(g_invalid_instance).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object - read only", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);

    property prop = prop_type.get_property("p2");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == true);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == false);
    CHECK(prop.get_type() == type::get<int>());
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    CHECK(prop.get_value(obj).is_type<int>() == true);
    CHECK(prop.get_value(obj).get_value<int>() == 12);

    // invalid invoke
    CHECK(prop.set_value(obj, 23) == false);
    CHECK(prop.get_value("wrong instance").is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object - bind as ptr", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);

    property prop = prop_type.get_property("p3");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == false);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == true);
    CHECK(prop.get_type() == type::get<std::vector<int>*>());
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    REQUIRE(prop.get_value(obj).is_type<std::vector<int>*>() == true);
    auto ptr = prop.get_value(obj).get_value<std::vector<int>*>();
    CHECK(ptr == &obj._p3);
    CHECK(prop.set_value(obj, ptr) == true);

    std::vector<int> some_vec(1, 12);
    CHECK(prop.set_value(obj, &some_vec) == true);
    CHECK(some_vec == obj._p3);

    // invalid invoke
    CHECK(prop.set_value(obj, "test") == false);
    CHECK(prop.set_value(g_invalid_instance, "test") == false);
    CHECK(prop.get_value(g_invalid_instance).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object - read only - bind as ptr", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);

    property prop = prop_type.get_property("p4");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == true);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == true);
    CHECK(prop.get_type() == type::get<const std::vector<int>*>());
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    CHECK(prop.get_value(obj).is_type<const std::vector<int>*>() == true);
    auto ptr = prop.get_value(obj).get_value<const std::vector<int>*>();
    CHECK(ptr == &obj._p4);
    CHECK(prop.set_value(obj, ptr) == false);

    std::vector<int> some_vec(1, 12);
    CHECK(prop.set_value(obj, &some_vec) == false);
    CHECK(some_vec != obj._p3);

    // invalid invoke
    CHECK(prop.set_value(obj, "test") == false);
    CHECK(prop.set_value(g_invalid_instance, "test") == false);
    CHECK(prop.get_value(g_invalid_instance).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object - as_reference_wrapper", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);

    property prop = prop_type.get_property("p5");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == false);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == true);
    CHECK(prop.get_type() == type::get<std::reference_wrapper<std::vector<int>>>());
    CHECK(prop.get_type().is_wrapper() == true);
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    REQUIRE(prop.get_value(obj).is_type<std::reference_wrapper<std::vector<int>>>() == true);
    auto value = prop.get_value(obj).get_value<std::reference_wrapper<std::vector<int>>>();
    CHECK(value.get() == obj._p3);
    CHECK(prop.set_value(obj, value) == true);

    // check array size
    variant var = prop.get_value(obj);
    CHECK(var.get_type().is_wrapper() == true);
    CHECK(var.get_type().get_wrapped_type().is_array() == true);
    auto array_view = var.create_array_view();
    CHECK(array_view.get_size() == 1000);
    CHECK(array_view.set_value(20, 42) == true);

    std::vector<int> some_vec(1, 12);
    CHECK(prop.set_value(obj, std::ref(some_vec)) == true);
    CHECK(some_vec == obj._p3);

    // invalid invoke
    CHECK(prop.set_value(obj, "test") == false);
    CHECK(prop.set_value(g_invalid_instance, "test") == false);
    CHECK(prop.get_value(g_invalid_instance).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - class object - read only - as_reference_wrapper", "[property]")
{
    property_member_obj_test obj;
    type prop_type = type::get(obj);

    property prop = prop_type.get_property("p6");
    REQUIRE(prop.is_valid() == true);

    // metadata
    CHECK(prop.is_readonly() == true);
    CHECK(prop.is_static() == false);
    CHECK(prop.is_array() == true);
    CHECK(prop.get_type() == type::get<std::reference_wrapper<const std::vector<int>>>());
    CHECK(prop.get_type().is_wrapper() == true);
    CHECK(prop.get_access_level() == rttr::access_levels::public_access);
    CHECK(prop.get_metadata("Description") == "Some Text");

    // invoke
    CHECK(prop.get_value(obj).is_type<std::reference_wrapper<const std::vector<int>>>() == true);
    auto value = prop.get_value(obj).get_value<std::reference_wrapper<const std::vector<int>>>();
    CHECK(value.get() == obj._p4);
    CHECK(prop.set_value(obj, value) == false);

    // check array size
    variant var = prop.get_value(obj);
    CHECK(var.get_type().is_wrapper() == true);
    CHECK(var.get_type().get_wrapped_type().is_array() == true);
    auto array_view = var.create_array_view();
    CHECK(array_view.get_size() == 50);
    CHECK(array_view.set_value(20, 42) == false);

    std::vector<int> some_vec(1, 12);
    CHECK(prop.set_value(obj, std::cref(some_vec)) == false);
    CHECK(some_vec != obj._p3);

    // invalid invoke
    CHECK(prop.set_value(obj, "test") == false);
    CHECK(prop.set_value(g_invalid_instance, "test") == false);
    CHECK(prop.get_value(g_invalid_instance).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - variant as property", "[property]")
{
    SECTION("Writable")
    {
        property_member_obj_test obj;
        type prop_type = type::get(obj);

        property prop = prop_type.get_property("p7");
        REQUIRE(prop.is_valid() == true);

        variant var = prop.get_value(obj);
        CHECK(obj._p7 == std::string("hello"));

        var = 23;
        prop.set_value(obj, var);

        CHECK(obj._p7.to_int() == 23);
    }

    SECTION("Read Only")
    {
        property_member_obj_test obj;
        type prop_type = type::get(obj);

        property prop = prop_type.get_property("p8");
        REQUIRE(prop.is_valid() == true);

        variant var = prop.get_value(obj);
        CHECK(obj._p8 == 23);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("property - raw pointer as property", "[property]")
{
    SECTION("Writable")
    {
        property_member_obj_test obj;
        type prop_type = type::get(obj);

        property prop = prop_type.get_property("p9");
        REQUIRE(prop.is_valid() == true);

        variant var = prop.get_value(obj);
        CHECK(obj._p9 == nullptr);

        int new_value = 23;
        prop.set_value(obj, &new_value);

        CHECK(obj._p9 == &new_value);
    }

    SECTION("Read Only")
    {
        property_member_obj_test obj;
        type prop_type = type::get(obj);

        property prop = prop_type.get_property("p10");
        REQUIRE(prop.is_valid() == true);

        variant var = prop.get_value(obj);
        CHECK(obj._p10 == &obj._p1);

        CHECK(var.get_type() == type::get<int*>());
        CHECK(obj._p10 == var.get_value<int*>());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
