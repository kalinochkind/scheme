#include <sstream>
#include <fstream>
#include "parser.h"
#include "std.h"

static FunctionPackage package(
    {
        {"display", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(!current_output_port->output_stream)
                throw eval_error("display: i/o error");
            (*current_output_port->output_stream) << l.front()->printable();
            return scheme_empty;
        }}},
        {"write", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(!current_output_port->output_stream)
                throw eval_error("write: i/o error");
            (*current_output_port->output_stream) << l.front()->external_repr();
            return scheme_empty;
        }}},
        {"read", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            Context dummy;
            if(!current_input_port->input_stream)
                throw eval_error("read: i/o error");
            auto x = read_object(*current_input_port->input_stream);
            if(x.result == parse_result_t::ERROR)
                throw eval_error("read: parse error: " + x.error);
            if(x.result == parse_result_t::END)
                return to_object(std::make_shared<SchemeSymbol>("eof"));
            return do_quote(x.node, dummy, 0).first;
        }}},
        {"current-input-port", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return to_object(current_input_port);
        }}},
        {"current-output-port", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return to_object(current_output_port);
        }}},
        {"set-current-input-port!", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePort>(l.front());
            if(!p || p->type == port_type_t::OUTPUT)
                throw eval_error("set-current-input-port!: an input port required");
            current_input_port = p;
            return scheme_empty;
        }}},
        {"set-current-output-port!", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePort>(l.front());
            if(!p || p->type == port_type_t::INPUT)
                throw eval_error("set-current-output-port!: an output port required");
            current_output_port = p;
            return scheme_empty;
        }}},
        {"close-input-port", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePort>(l.front());
            if(!p || p->type == port_type_t::OUTPUT)
                throw eval_error("close-input-port: an input port required");
            p->close_input();
            return scheme_empty;
        }}},
        {"close-output-port", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePort>(l.front());
            if(!p || p->type == port_type_t::INPUT)
                throw eval_error("close-output-port: an output port required");
            p->close_output();
            return scheme_empty;
        }}},
        {"open-input-file", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto s = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!s)
                throw eval_error("open-input-file: a string required");
            auto is = std::make_shared<std::ifstream>(s->value);
            if(!is->is_open())
                throw eval_error("open-input-file: failed to open");
            auto f = std::make_shared<SchemeFilePort>(port_type_t::INPUT);
            f->input_stream = is;
            SchemeFilePort::open_files.insert(f.get());
            return to_object(f);
        }}},
        {"open-output-file", {1, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto s = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!s)
                throw eval_error("open-output-file: a string required");
            bool append = l.size() == 2 && l.back()->to_bool();
            auto os = std::make_shared<std::ofstream>(s->value, append ? std::ofstream::app : std::ofstream::out);
            if(!os->is_open())
                throw eval_error("open-output-file: failed to open");
            auto f = std::make_shared<SchemeFilePort>(port_type_t::OUTPUT);
            f->output_stream = os;
            SchemeFilePort::open_files.insert(f.get());
            return to_object(f);
        }}},
        {"open-i/o-file", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto s = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!s)
                throw eval_error("open-i/o-file: a string required");
            auto fs = std::make_shared<std::fstream>(s->value);
            if(!fs->is_open())
                throw eval_error("open-i/o-file: failed to open");
            auto f = std::make_shared<SchemeFilePort>(port_type_t::IO);
            f->input_stream = fs;
            f->output_stream = fs;
            SchemeFilePort::open_files.insert(f.get());
            return to_object(f);
        }}},
        {"close-all-open-files", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            SchemeFilePort::close_all_files();
            return scheme_empty;
        }}},
        {"open-input-string", {1, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto s = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!s)
                throw eval_error("open-input-string: a string required");
            long long start = 0, end = s->value.length();
            if(l.size() >= 2)
            {
                auto pa = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
                if(!pa)
                    throw eval_error("open-input-string: invalid range");
                start = pa->value;
                if(l.size() == 3)
                {
                    auto pb = std::dynamic_pointer_cast<SchemeInt>(l.back());
                    if(!pb)
                        throw eval_error("open-input-string: invalid range");
                    end = pb->value;
                }
            }
            if(start < 0 || end > (long long) s->value.length() || start > end)
                throw eval_error("open-input-string: invalid range");
            auto str = s->value.substr(start, end - start);
            auto is = std::make_shared<std::istringstream>(str);
            auto f = std::make_shared<SchemeStringPort>(port_type_t::INPUT);
            f->input_stream = is;
            return to_object(f);
        }}},
        {"open-output-string", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            auto os = std::make_shared<std::ostringstream>();
            auto f = std::make_shared<SchemeStringPort>(port_type_t::OUTPUT);
            f->output_stream = os;
            return to_object(f);
        }}},
        {"get-output-string", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeStringPort>(l.front());
            if(!p || p->type == port_type_t::INPUT)
                throw eval_error("get-output-string: an output string port required");
            auto os = std::dynamic_pointer_cast<std::ostringstream>(p->output_stream);
            if(!os)
                throw eval_error("get-output-string: invalid port");
            auto s = std::make_shared<SchemeString>(os->str());
            return to_object(s);
        }}},
    }
);