#include <iostream>
#include <sgfx/ppm.hpp>
#include <vector>

#define let auto /* Pure provocation with respect to my dire love to F# & my hate to C++ auto keyword. */

namespace sgfx::ppm {

using namespace std;

canvas Parser::parseString(std::string const& data)
{
    offset_ = 0;
    source_ = &data;
    consumeToken();  // initialize tokenizer

    parseMagic();
    let dim = parseDimension();

    /*let maximumColorValue = */ parseNumber();

    let pixels = vector<color::rgb_color>{};
    pixels.reserve(dim.width * dim.height);

    while (!eof())
        pixels.emplace_back(parsePixel());

    return canvas{dim, move(pixels)};
}

void Parser::fatalSyntaxError(std::string const& diagnosticMessage)
{
    throw FileFormatError{diagnosticMessage};
}

char Parser::currentChar() const noexcept
{
    return offset_ < source_->size() ? source_->at(offset_) : '\0';
}

char Parser::peekChar() const noexcept
{
    return offset_ + 1 < source_->size() ? source_->at(offset_ + 1) : '\0';
}

char Parser::nextChar() noexcept
{
    if (offset_ < source_->size())
        ++offset_;

    return currentChar();
}

Parser::TokenInfo Parser::consumeToken()
{
    let const consumeOneToken = [this]() -> TokenInfo {
        // consume any leading spaces
        while (!eof() && isspace(currentChar()))
            nextChar();

        if (eof())
            return TokenInfo{Token::Eof, "EOF"};

        if (currentChar() == '#')  // parse comment
        {
            nextChar();  // skip '#'
            let text = string{};
            while (!eof() && currentChar() != '\n')
            {
                text += currentChar();
                nextChar();
            }
            return TokenInfo{Token::Comment, text};
        }

        if (currentChar() == 'P' && peekChar() == '3')
        {
            nextChar();  // skip P
            nextChar();  // skip 3
            return TokenInfo{Token::Magic, "P3"};
        }

        if (isdigit(currentChar()))
        {
            let literal = string{};
            literal += currentChar();
            while (isdigit(nextChar()))
                literal += currentChar();
            return TokenInfo{Token::Number, literal};
        }

        return TokenInfo{Token::Invalid, string(1, currentChar())};
    };

    do
        currentToken_ = consumeOneToken();
    while (currentToken_.token == Token::Comment);

    return currentToken_;
}

string Parser::consumeToken(Token token)
{
    if (currentToken().token != token)
        fatalSyntaxError("Unexpected token.");

    let literal = currentToken().literal;
    consumeToken();
    return literal;
}

string Parser::parseMagic()
{
    if (currentToken().token != Token::Magic)
        fatalSyntaxError("Expected Magic.");

    let id = currentToken().literal;
    consumeToken();
    return id;
}

dimension Parser::parseDimension()
{
    let width = stoi(consumeToken(Token::Number));
    let height = stoi(consumeToken(Token::Number));
    return dimension{width, height};
}

color::rgb_color Parser::parsePixel()
{
    let const parsePixel = [this]() -> uint8_t {
        int value = parseNumber();
        if (value >= 0 && value <= 255)
            return static_cast<uint8_t>(value);
        fatalSyntaxError("Pixel value out of range.");
        return 0;  // never reached
    };

    let const red = parsePixel();
    let const green = parsePixel();
    let const blue = parsePixel();
    let const color = color::rgb_color{red, green, blue};

    return color;
}

int Parser::parseNumber()
{
    if (currentToken().token != Token::Number)
        fatalSyntaxError("Expected a number.");

    let number = stoi(currentToken().literal);
    consumeToken();
    return number;
}

}  // namespace sgfx::ppm
