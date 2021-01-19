#include "Exception.h"

namespace ham
{
    Exception::Exception(std::string msg)
            : msg_(std::move(msg)),
              stack_(fillStack())
    {}
    
    std::string Exception::fillStack() const noexcept
    {
        std::string demangled_stack;
        int max_frames = 200;
        void* funcPtrs[max_frames];
        // retrieve current stack addresses
        int num = ::backtrace(funcPtrs, max_frames);
        char** funcNames = ::backtrace_symbols(funcPtrs, num); // malloc! remember to free
        
        if(funcNames)
        {
            size_t size = 256;
            char* curFuncName = static_cast<char*>(malloc(size));
            
            for(int i = 1;i < num;++i)
            {
                printf("%s\n", funcNames[i]);
                char* begin_name = 0, *begin_offset = 0, *end_offset = 0;
                for(char* pos = funcNames[i];*pos;++pos)
                {
                    if(*pos == '(')
                        begin_name = pos;
                    else if(*pos == '+')
                        begin_offset = pos;
                    else if(*pos == ')' && begin_offset)
                    {
                        end_offset = pos;
                        break;
                    }
                }        
                
                *begin_name++ = '\0';
	            *begin_offset++ = '\0';
	            *end_offset = '\0';

                printf("%s\n", begin_name);

                int status = 0;
                char* res = abi::__cxa_demangle(begin_name, curFuncName, &size, &status);
                if(status == 0)
                {
                    curFuncName = res; 
                    demangled_stack.append(curFuncName);
                    demangled_stack.push_back('\n');
                }
                else
                {
                    demangled_stack.append(funcNames[i]);
                }
                
            }
            free(curFuncName);
        }
        free(funcNames);
        printf("end demangle\n");
        return demangled_stack;
    }
}