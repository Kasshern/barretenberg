#include <barretenberg/noir/ast.hpp>
#include <barretenberg/noir/compiler/compiler.hpp>
#include <barretenberg/noir/parse.hpp>
#include <fstream>
#include <gtest/gtest.h>

using namespace barretenberg;
using namespace noir::parser;
using namespace noir::code_gen;

TEST(noir, format_string)
{
    EXPECT_EQ(format("hello %s %d", "world", 123), "hello world 123");
}

TEST(noir, parse_fails)
{
    EXPECT_THROW(parse("1 + 2; blah"), std::runtime_error);
}

TEST(noir, uint_sizes)
{
    auto ast = parse("          \n\
        uint2 my_int2 = 0;      \n\
        uint3 my_int3 = 0;      \n\
        uint32 my_int32 = 0;    \n\
        uint64 my_int64 = 0;    \n\
    ");

    auto type_id = boost::get<noir::ast::variable_declaration>(ast[0]).type;
    auto int_type = boost::get<noir::ast::int_type>(type_id.type);
    EXPECT_EQ(int_type.size, 2UL);
}

TEST(noir, uint1_fail)
{
    EXPECT_THROW(parse("uint1 my_int1 = 0;"), std::runtime_error);
}

TEST(noir, uint65_fail)
{
    EXPECT_THROW(parse("uint65 my_int65 = 0;"), std::runtime_error);
}

TEST(noir, function_definition)
{
    parse("uint32 my_function(uint32 arg1, bool arg2) {}");
}

TEST(noir, function_call)
{
    parse("bool x = my_function(arg1, 3+5+(x));");
}

TEST(noir, array_variable_definition)
{
    parse("uint32[4] my_var = [0x1, 0x12, 0x123, 0x1234];");
}

TEST(noir, array_expressions)
{
    parse_function_statements("uint32[4] my_var = [func_call(), 13, true];");
}

TEST(noir, array_index)
{
    parse_function_statements("my_var = some_array[5*3][1+2];");
}

TEST(noir, unary)
{
    parse_function_statements("my_var = !x;");
}

/*
TEST(noir, function_copy_by_value)
{
    // TODO: Include mutable keyword on b declaration.
    std::string code = "            \n\
        bool[2] main(bool[2] a) {   \n\
            bool[2] b = a;          \n\
            b[0] = true;            \n\
            return b;               \n\
        }                           \n\
    ";
    auto ast = parse(code);

    auto composer = Composer();
    auto compiler = Compiler(composer);
    std::vector<var_t> inputs = { std::vector<var_t>(2, bool_t(witness_t(&composer, false))) };
    auto r = compiler.start(ast, inputs);
    EXPECT_EQ(boost::get<bool_t>(inputs[0].value).value(), false);
}
*/

TEST(noir, bool_circuit)
{
    std::string code = "                      \n\
    bool main(bool a, bool b) {               \n\
      a = a ^ b;         // a = 1             \n\
      b = !b;            // b = 1 (witness 0) \n\
      bool c = (a == b); // c = 1             \n\
      bool d;            // d = ?             \n\
      d = false;         // d = 0             \n\
      bool e = a | d;    // e = 1 = a         \n\
      bool f = e ^ b;    // f = 0             \n\
      d = (!f) & a;      // d = 1             \n\
    }                                         \n\
    ";
    auto ast = parse(code);

    auto composer = Composer();
    auto compiler = Compiler(composer);
    std::vector<var_t> inputs = { bool_t(witness_t(&composer, true)), bool_t(witness_t(&composer, false)) };
    auto r = compiler.start(ast, inputs);
    auto prover = std::move(r.second);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[0]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[0]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[0]), { { 1, 0, 0, 0 } }), true);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[1]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[1]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[1]), { { 0, 0, 0, 0 } }), true);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[2]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[2]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[2]), { { 1, 0, 0, 0 } }), true);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[3]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[3]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[3]), { { 1, 0, 0, 0 } }), true);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[4]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[4]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[4]), { { 0, 0, 0, 0 } }), true);

    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_l[5]), { { 0, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_r[5]), { { 1, 0, 0, 0 } }), true);
    EXPECT_EQ(fr::eq(fr::from_montgomery_form(prover.w_o[5]), { { 1, 0, 0, 0 } }), true);

    EXPECT_EQ(prover.n, 8UL);
}

TEST(noir, sha256)
{
    std::ifstream file("../test/noir/sha256.noir");
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto ast = parse(code);

    auto composer = Composer();

    uint32_t hex_input[] = {
        0xa6e53dc5, 0x295acbff, 0xf63fccde, 0x378e1cbe, 0xbe9f04de, 0x8e35c7ce, 0xfd9105e7, 0x391f8e34,
        0x56e08d13, 0x6ed204e6, 0xc7d80b22, 0xa1660521, 0xc2320131, 0xd5ab1f8e, 0x180ede60, 0x6574be20,
    };

    // uint32_t expected[] = {
    //     0x70f94a7c, 0xc60c3099, 0x93a27a6e, 0xa2b269a1, 0xf8db998a, 0xce7bc970, 0xeb4c360e, 0xc6042b88,
    // };

    std::vector<var_t> inputs(16, uint32());
    for (size_t i = 0; i < 16; ++i) {
        inputs[i] = uint32(witness_t(&composer, hex_input[i]));
    }

    std::vector<var_t> args = { var_t(inputs) };

    std::cout << "circuit inputs " << inputs << std::endl;

    auto compiler = Compiler(composer);
    auto prover = compiler.start(ast, args);

    /*
        std::array<uint32, 8> outputs = plonk::stdlib::sha256(inputs);

        printf("composer gates = %lu\n", composer.adjusted_n);
        waffle::Verifier verifier = waffle::preprocess(prover);

        waffle::plonk_proof proof = prover.construct_proof();

        bool result = verifier.verify_proof(proof);
        EXPECT_EQ(result, true);
        */
}