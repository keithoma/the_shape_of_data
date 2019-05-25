#include <sgfx/canvas.hpp>
#include <sgfx/color.hpp>
#include <sgfx/primitive_types.hpp>

namespace sgfx::ppm {

class Parser {
  public:
    canvas parseString(std::string const& data);

  private:
    enum class Token {
        Invalid,
        Number,
        Comment,
        Magic,
        Eof,
    };

    struct TokenInfo {
        Token token;
        std::string literal;
    };

    class FileFormatError : public std::runtime_error {
      public:
        FileFormatError(std::string const& msg) : std::runtime_error{msg} {}
    };

    void fatalSyntaxError(std::string const& diagnosticMessage);

    // lexical analysis
    bool eof() const noexcept { return !source_ || offset_ >= source_->size() || currentToken_.token == Token::Eof; }
    char currentChar() const noexcept;
    char peekChar() const noexcept;
    char nextChar();
    TokenInfo currentToken() const noexcept { return currentToken_; }
    TokenInfo consumeToken();
    TokenInfo consumeTokenInternal();
    std::string consumeToken(Token token);

    // syntactical analysis
    std::string parseMagic();
    dimension parseDimension();
    color::rgb_color parsePixel();
    int parseNumber();

  private:
    std::string const* source_ = nullptr;
    size_t offset_{0};
    TokenInfo currentToken_{Token::Invalid, ""};
};

}  // namespace sgfx::ppm
