// LICENSE AT END OF FILE (MIT).

#define MD_FUNCTION_IMPL MD_FUNCTION
#define MD_PRIVATE_FUNCTION_IMPL MD_FUNCTION_IMPL

//~

// NOTE(allen): Review @rjf; Building in C++
// While very latest version of C++ have designated initializers
// I would like to be able to build on more simple versions, so I
// ditched the designated initializers in favor of the extra work
// of maintaining order based initializers.
static MD_Node _md_nil_node =
{
    &_md_nil_node,         // next
    &_md_nil_node,         // prev
    &_md_nil_node,         // parent
    &_md_nil_node,         // first_child
    &_md_nil_node,         // last_child
    &_md_nil_node,         // first_tag
    &_md_nil_node,         // last_tag
    MD_NodeKind_Nil,       // kind
    0,                     // flags
    {0},                   // string
    {0},                   // whole_string
    0xdeadffffffffffull,
    {(MD_u8*)"`NIL DD NODE`", 13},
    0,
    0,
};

MD_PRIVATE_FUNCTION_IMPL void
_MD_MemoryZero(void *memory, MD_u64 size)
{
    memset(memory, 0, size);
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_MemoryCopy(void *dest, void *src, MD_u64 size)
{
    memcpy(dest, src, size);
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_WriteStringToBuffer(MD_String8 string, MD_u64 max, void *dest)
{
    MD_u64 write_size = string.size;
    if(write_size > max-1) write_size = max-1;
    _MD_MemoryCopy(dest, string.str, write_size);
    ((MD_u8 *)dest)[write_size] = 0;
}

MD_PRIVATE_FUNCTION_IMPL void *
_MD_AllocZero(void *ctx, MD_u64 size)
{
#if !defined(MD_IMPL_Alloc)
# error Missing implementation detail MD_IMPL_Alloc
#else
    void *result = MD_IMPL_Alloc(ctx, size);
    _MD_MemoryZero(result, size);
    return(result);
#endif
}

#define _MD_PushArray(ctx, type, count) (type *)_MD_AllocZero((ctx), sizeof(type)*(count))

MD_PRIVATE_FUNCTION_IMPL void*
_MD_GetCtx(void)
{
#if !defined(MD_IMPL_GetCtx)
    return(0);
#else
    return(MD_IMPL_GetCtx());
#endif
}


MD_FUNCTION_IMPL MD_b32
MD_CharIsAlpha(MD_u8 c)
{
    return MD_CharIsAlphaUpper(c) || MD_CharIsAlphaLower(c);
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaUpper(MD_u8 c)
{
    return c >= 'A' && c <= 'Z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaLower(MD_u8 c)
{
    return c >= 'a' && c <= 'z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsDigit(MD_u8 c)
{
    return (c >= '0' && c <= '9');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSymbol(MD_u8 c)
{
    return (c == '~' || c == '!' || c == '@' || c == '#' || c == '$' ||
            c == '%' || c == '^' || c == '&' || c == '*' || c == '(' ||
            c == ')' || c == '-' || c == '=' || c == '+' || c == '[' ||
            c == ']' || c == '{' || c == '}' || c == ':' || c == ';' ||
            c == ',' || c == '<' || c == '.' || c == '>' || c == '/' ||
            c == '?' || c == '|' || c == '\\');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSpace(MD_u8 c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v';
}

MD_FUNCTION_IMPL MD_u8
MD_CharToUpper(MD_u8 c)
{
    return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CharToLower(MD_u8 c)
{
    return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CorrectSlash(MD_u8 c)
{
    return (c == '\\' ? '/' : c);
}

MD_FUNCTION_IMPL MD_String8
MD_S8(MD_u8 *str, MD_u64 size)
{
    MD_String8 string;
    string.str = str;
    string.size = size;
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_S8Range(MD_u8 *str, MD_u8 *opl)
{
    MD_String8 string;
    string.str = str;
    string.size = (MD_u64)(opl - str);
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSubstring(MD_String8 str, MD_u64 min, MD_u64 max)
{
    if(max > str.size)
    {
        max = str.size;
    }
    if(min > str.size)
    {
        min = str.size;
    }
    if(min > max)
    {
        MD_u64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSkip(MD_String8 str, MD_u64 min)
{
    return MD_StringSubstring(str, min, str.size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringChop(MD_String8 str, MD_u64 nmax)
{
    return MD_StringSubstring(str, 0, str.size - nmax);
}

MD_FUNCTION_IMPL MD_String8
MD_StringPrefix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, 0, size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringSuffix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, str.size - size, size);
}

MD_FUNCTION_IMPL MD_b32
MD_StringMatch(MD_String8 a, MD_String8 b, MD_StringMatchFlags flags)
{
    int result = 0;
    if(a.size == b.size || flags & MD_StringMatchFlag_RightSideSloppy)
    {
        result = 1;
        for(MD_u64 i = 0; i < a.size; i += 1)
        {
            MD_b32 match = (a.str[i] == b.str[i]);
            if(flags & MD_StringMatchFlag_CaseInsensitive)
            {
                match |= (MD_CharToLower(a.str[i]) == MD_CharToLower(b.str[i]));
            }
            if(flags & MD_StringMatchFlag_SlashInsensitive)
            {
                match |= (MD_CorrectSlash(a.str[i]) == MD_CorrectSlash(b.str[i]));
            }
            if(match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_u64
MD_FindSubstring(MD_String8 str, MD_String8 substring, MD_u64 start_pos, MD_StringMatchFlags flags)
{
    MD_b32 found = 0;
    MD_u64 found_idx = str.size;
    for(MD_u64 i = start_pos; i < str.size; i += 1)
    {
        if(i + substring.size <= str.size)
        {
            MD_String8 substr_from_str = MD_StringSubstring(str, i, i+substring.size);
            if(MD_StringMatch(substr_from_str, substring, flags))
            {
                found_idx = i;
                found = 1;
                if(!(flags & MD_StringMatchFlag_FindLast))
                {
                    break;
                }
            }
        }
    }
    return found_idx;
}

MD_FUNCTION_IMPL MD_u64
MD_FindLastSubstring(MD_String8 str, MD_String8 substring, MD_StringMatchFlags flags)
{
    return MD_FindSubstring(str, substring, 0, flags | MD_StringMatchFlag_FindLast);
}

MD_FUNCTION_IMPL MD_String8
MD_TrimExtension(MD_String8 string)
{
    MD_u64 period_pos = MD_FindLastSubstring(string, MD_S8Lit("."), 0);
    if(period_pos < string.size)
    {
        string.size = period_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_TrimFolder(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindLastSubstring(string, MD_S8Lit("/"), MD_StringMatchFlag_SlashInsensitive);
    if(slash_pos < string.size)
    {
        string.str += slash_pos+1;
        string.size -= slash_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_ExtensionFromPath(MD_String8 string)
{
    MD_u64 period_pos = MD_FindLastSubstring(string, MD_S8Lit("."), 0);
    if(period_pos < string.size)
    {
        string.str += period_pos+1;
        string.size -= period_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_FolderFromPath(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindLastSubstring(string, MD_S8Lit("/"), MD_StringMatchFlag_SlashInsensitive);
    if(slash_pos < string.size)
    {
        string.size = slash_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringCopy(MD_String8 string)
{
    MD_String8 res;
    res.size = string.size;
    res.str = _MD_PushArray(_MD_GetCtx(), MD_u8, string.size+1);
    _MD_MemoryCopy(res.str, string.str, string.size);
    return res;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringFV(char *fmt, va_list args)
{
    MD_String8 result = {0};
    va_list args2;
    va_copy(args2, args);
    MD_u64 needed_bytes = vsnprintf(0, 0, fmt, args)+1;
    result.str = _MD_PushArray(_MD_GetCtx(), MD_u8, needed_bytes);
    result.size = needed_bytes-1;
    vsnprintf((char*)result.str, needed_bytes, fmt, args2);
    return result;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringF(char *fmt, ...)
{
    MD_String8 result = {0};
    va_list args;
    va_start(args, fmt);
    result = MD_PushStringFV(fmt, args);
    va_end(args);
    return result;
}

MD_FUNCTION_IMPL void
MD_PushStringToList(MD_String8List *list, MD_String8 string)
{
    list->node_count += 1;
    list->total_size += string.size;
    
    MD_String8Node *node = _MD_PushArray(_MD_GetCtx(), MD_String8Node, 1);
    node->next = 0;
    node->string = string;
    if(list->last == 0)
    {
        list->first = list->last = node;
    }
    else
    {
        list->last->next = node;
        list->last = list->last->next;
    }
}

MD_FUNCTION_IMPL void
MD_PushStringListToList(MD_String8List *list, MD_String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    _MD_MemoryZero(to_push, sizeof(*to_push));
}

MD_FUNCTION_IMPL MD_String8List
MD_SplitString(MD_String8 string, int split_count, MD_String8 *splits)
{
    MD_String8List list = {0};
    
    MD_u64 split_start = 0;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        MD_b32 was_split = 0;
        for(int split_idx = 0; split_idx < split_count; split_idx += 1)
        {
            MD_b32 match = 0;
            if(i + splits[split_idx].size <= string.size)
            {
                match = 1;
                for(MD_u64 split_i = 0; split_i < splits[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splits[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                MD_String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_PushStringToList(&list, split_string);
                split_start = i + splits[split_idx].size;
                i += splits[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            MD_String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_PushStringToList(&list, split_string);
            break;
        }
    }
    
    return list;
}

MD_FUNCTION_IMPL MD_String8List
MD_SplitStringByString(MD_String8 string, MD_String8 split)
{
    return MD_SplitString(string, 1, &split);
}

MD_FUNCTION_IMPL MD_String8List
MD_SplitStringByCharacter(MD_String8 string, MD_u8 character)
{
    return MD_SplitStringByString(string, MD_S8(&character, 1));
}

MD_FUNCTION_IMPL MD_String8
MD_JoinStringList(MD_String8List list)
{
    MD_String8 string = {0};
    string.size = list.total_size;
    string.str = _MD_PushArray(_MD_GetCtx(), MD_u8, string.size);
    MD_u64 write_pos = 0;
    for(MD_String8Node *node = list.first; node; node = node->next)
    {
        _MD_MemoryCopy(string.str + write_pos, node->string.str, node->string.size);
        write_pos += node->string.size;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_JoinStringListWithSeparator(MD_String8List list, MD_String8 separator)
{
    if (list.node_count == 0)
    {
        return MD_S8Lit("");
    }
    MD_String8 string = {0};
    string.size = list.total_size + (list.node_count - 1)*separator.size;
    string.str = _MD_PushArray(_MD_GetCtx(), MD_u8, string.size);
    MD_u64 write_pos = 0;
    for(MD_String8Node *node = list.first; node; node = node->next)
    {
        _MD_MemoryCopy(string.str + write_pos, node->string.str, node->string.size);
        write_pos += node->string.size;
        if (node != list.last){
            _MD_MemoryCopy(string.str + write_pos, separator.str, separator.size);
            write_pos += separator.size;
        }
    }
    return string;
}

MD_FUNCTION_IMPL MD_i64
MD_I64FromString(MD_String8 string)
{
    char str[64];
    _MD_WriteStringToBuffer(string, sizeof(str), str);
    return atoll(str);
}

MD_FUNCTION_IMPL MD_f64
MD_F64FromString(MD_String8 string)
{
    char str[64];
    _MD_WriteStringToBuffer(string, sizeof(str), str);
    return atof(str);
}

MD_FUNCTION_IMPL MD_u64
MD_HashString(MD_String8 string)
{
    MD_u64 result = 5381;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        result = ((result << 5) + result) + string.str[i];
    }
    return result;
}

MD_FUNCTION_IMPL MD_u64
MD_CalculateCStringLength(char *cstr)
{
    MD_u64 i = 0;
    for(; cstr[i]; i += 1);
    return i;
}

MD_FUNCTION_IMPL MD_String8
MD_StyledStringFromString(MD_String8 string, MD_WordStyle word_style, MD_String8 separator)
{
    MD_String8 result = {0};
    
    MD_String8List words = {0};
    
    MD_b32 break_on_uppercase = 0;
    {
        break_on_uppercase = 1;
        for(MD_u64 i = 0; i < string.size; i += 1)
        {
            if(!MD_CharIsAlpha(string.str[i]) && !MD_CharIsDigit(string.str[i]))
            {
                break_on_uppercase = 0;
                break;
            }
        }
    }
    
    MD_b32 making_word = 0;
    MD_String8 word = {0};
    
    for(MD_u64 i = 0; i < string.size;)
    {
        if(making_word)
        {
            if((break_on_uppercase && MD_CharIsAlphaUpper(string.str[i])) ||
               string.str[i] == '_' || MD_CharIsSpace(string.str[i]) ||
               i == string.size - 1)
            {
                if(i == string.size - 1)
                {
                    word.size += 1;
                }
                making_word = 0;
                MD_PushStringToList(&words, word);
            }
            else
            {
                word.size += 1;
                i += 1;
            }
        }
        else
        {
            if(MD_CharIsAlpha(string.str[i]))
            {
                making_word = 1;
                word.str = string.str + i;
                word.size = 1;
            }
            i += 1;
        }
    }
    
    result.size = words.total_size;
    if(words.node_count > 1)
    {
        result.size += separator.size*(words.node_count-1);
    }
    result.str = _MD_PushArray(_MD_GetCtx(), MD_u8, result.size);
    
    {
        MD_u64 write_pos = 0;
        for(MD_String8Node *node = words.first; node; node = node->next)
        {
            
            // NOTE(rjf): Write word string to result.
            {
                _MD_MemoryCopy(result.str + write_pos, node->string.str, node->string.size);
                
                // NOTE(rjf): Transform string based on word style.
                switch(word_style)
                {
                    case MD_WordStyle_UpperCamelCase:
                    {
                        result.str[write_pos] = MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCamelCase:
                    {
                        result.str[write_pos] = node == words.first ? MD_CharToLower(result.str[write_pos]) : MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_UpperCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToUpper(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    default: break;
                }
                
                write_pos += node->string.size;
            }
            
            if(node->next)
            {
                _MD_MemoryCopy(result.str + write_pos, separator.str, separator.size);
                write_pos += separator.size;
            }
        }
    }
    
    return result;
}

MD_FUNCTION_IMPL MD_String8
MD_StringFromNodeKind(MD_NodeKind kind)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeKind enum.
    static char *cstrs[MD_NodeKind_MAX] =
    {
        "Nil",
        "File",
        "Label",
        "UnnamedSet",
        "Tag",
    };
    return MD_S8CString(cstrs[kind]);
}

MD_FUNCTION_IMPL MD_String8List
MD_StringListFromNodeFlags(MD_NodeFlags flags)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeFlags enum.
    static char *flag_cstrs[] =
    {
        "ParenLeft",
        "ParenRight",
        "BracketLeft",
        "BracketRight",
        "BraceLeft",
        "BraceRight",
        
        "BeforeSemicolon",
        "BeforeComma",
        
        "AfterSemicolon",
        "AfterComma",
        
        "Numeric",
        "Identifier",
        "StringLiteral",
        "CharLiteral",
    };
    
    MD_String8List list = {0};
    MD_u64 bits = sizeof(flags) * 8;
    for(MD_u64 i = 0; i < bits && i < MD_ArrayCount(flag_cstrs); i += 1)
    {
        if(flags & (1ull << i))
        {
            MD_PushStringToList(&list, MD_S8CString(flag_cstrs[i]));
        }
    }
    return list;
}

////////////////////////////////
// NOTE(allen): Unicode

MD_GLOBAL MD_u8 dd_utf8_class[32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf8(MD_u8 *str, MD_u64 max)
{
#define MD_bitmask1 0x01
#define MD_bitmask2 0x03
#define MD_bitmask3 0x07
#define MD_bitmask4 0x0F
#define MD_bitmask5 0x1F
#define MD_bitmask6 0x3F
#define MD_bitmask7 0x7F
#define MD_bitmask8 0xFF
#define MD_bitmask9  0x01FF
#define MD_bitmask10 0x03FF
    
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    MD_u8 byte = str[0];
    MD_u8 byte_class = dd_utf8_class[byte >> 3];
    switch (byte_class)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= max)
            {
                MD_u8 cont_byte = str[1];
                if (dd_utf8_class[cont_byte >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask5) << 6;
                    result.codepoint |=  (cont_byte & MD_bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= max)
            {
                MD_u8 cont_byte[2] = {str[1], str[2]};
                if (dd_utf8_class[cont_byte[0] >> 3] == 0 &&
                    dd_utf8_class[cont_byte[1] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask4) << 12;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 6);
                    result.codepoint |=  (cont_byte[1] & MD_bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= max)
            {
                MD_u8 cont_byte[3] = {str[1], str[2], str[3]};
                if (dd_utf8_class[cont_byte[0] >> 3] == 0 &&
                    dd_utf8_class[cont_byte[1] >> 3] == 0 &&
                    dd_utf8_class[cont_byte[2] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask3) << 18;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 12);
                    result.codepoint |= ((cont_byte[1] & MD_bitmask6) <<  6);
                    result.codepoint |=  (cont_byte[2] & MD_bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    
    return(result);
}

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf16(MD_u16 *out, MD_u64 max)
{
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    result.codepoint = out[0];
    result.advance = 1;
    if (1 < max && 0xD800 <= out[0] && out[0] < 0xDC00 && 0xDC00 <= out[1] && out[1] < 0xE000)
    {
        result.codepoint = ((out[0] - 0xD800) << 10) | (out[1] - 0xDC00);
        result.advance = 2;
    }
    return(result);
}

MD_FUNCTION MD_u32
MD_Utf8FromCodepoint(MD_u8 *out, MD_u32 codepoint)
{
#define MD_bit8 0x80
    MD_u32 advance = 0;
    if (codepoint <= 0x7F)
    {
        out[0] = (MD_u8)codepoint;
        advance = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (MD_bitmask2 << 6) | ((codepoint >> 6) & MD_bitmask5);
        out[1] = MD_bit8 | (codepoint & MD_bitmask6);
        advance = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (MD_bitmask3 << 5) | ((codepoint >> 12) & MD_bitmask4);
        out[1] = MD_bit8 | ((codepoint >> 6) & MD_bitmask6);
        out[2] = MD_bit8 | ( codepoint       & MD_bitmask6);
        advance = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (MD_bitmask4 << 3) | ((codepoint >> 18) & MD_bitmask3);
        out[1] = MD_bit8 | ((codepoint >> 12) & MD_bitmask6);
        out[2] = MD_bit8 | ((codepoint >>  6) & MD_bitmask6);
        out[3] = MD_bit8 | ( codepoint        & MD_bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return(advance);
}

MD_FUNCTION MD_u32
MD_Utf16FromCodepoint(MD_u16 *out, MD_u32 codepoint)
{
    MD_u32 advance = 1;
    if (codepoint == ~((MD_u32)0))
    {
        out[0] = (MD_u16)'?';
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (MD_u16)codepoint;
    }
    else
    {
        MD_u64 v = codepoint - 0x10000;
        out[0] = 0xD800 + (v >> 10);
        out[1] = 0xDC00 + (v & MD_bitmask10);
        advance = 2;
    }
    return(advance);
}

MD_FUNCTION MD_String8
MD_S8FromS16(MD_String16 in)
{
    MD_u64 cap = in.size*3;
    MD_u8 *str = _MD_PushArray(_MD_GetCtx(), MD_u8, cap + 1);
    MD_u16 *ptr = in.str;
    MD_u16 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String16
MD_S16FromS8(MD_String8 in)
{
    MD_u64 cap = in.size*2;
    MD_u16 *str = _MD_PushArray(_MD_GetCtx(), MD_u16, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    MD_String16 result = {str, size};
    return(result);
}

MD_FUNCTION MD_String8
MD_S8FromS32(MD_String32 in)
{
    MD_u64 cap = in.size*4;
    MD_u8 *str = _MD_PushArray(_MD_GetCtx(), MD_u8, cap + 1);
    MD_u32 *ptr = in.str;
    MD_u32 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl; ptr += 1)
    {
        size += MD_Utf8FromCodepoint(str + size, *ptr);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String32
MD_S32FromS8(MD_String8 in)
{
    MD_u64 cap = in.size;
    MD_u32 *str = _MD_PushArray(_MD_GetCtx(), MD_u32, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    MD_String32 result = {str, size};
    return(result);
}


MD_PRIVATE_FUNCTION_IMPL void
_MD_NodeTable_Initialize(MD_NodeTable *table)
{
    if(table->table_size == 0)
    {
        table->table_size = 4096;
        table->table = _MD_PushArray(_MD_GetCtx(), MD_NodeTableSlot *, table->table_size);
    }
}

MD_FUNCTION_IMPL MD_NodeTableSlot *
MD_NodeTable_Lookup(MD_NodeTable *table, MD_String8 string)
{
    _MD_NodeTable_Initialize(table);
    
    MD_NodeTableSlot *slot = 0;
    MD_u64 hash = MD_HashString(string);
    MD_u64 index = hash % table->table_size;
    for(MD_NodeTableSlot *candidate = table->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    return slot;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeTable_Insert(MD_NodeTable *table, MD_NodeTableCollisionRule collision_rule, MD_String8 string, MD_Node *node)
{
    _MD_NodeTable_Initialize(table);
    
    MD_NodeTableSlot *slot = 0;
    MD_u64 hash = MD_HashString(string);
    MD_u64 index = hash % table->table_size;
    
    for(MD_NodeTableSlot *candidate = table->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    
    if(slot == 0 || (slot != 0 && collision_rule == MD_NodeTableCollisionRule_Chain))
    {
        slot = _MD_PushArray(_MD_GetCtx(), MD_NodeTableSlot, 1);
        if(slot)
        {
            slot->next = 0;
            if(table->table[index])
            {
                for(MD_NodeTableSlot *old_slot = table->table[index]; old_slot; old_slot = old_slot->next)
                {
                    if(old_slot->next == 0)
                    {
                        old_slot->next = slot;
                        break;
                    }
                }
            }
            else
            {
                table->table[index] = slot;
            }
        }
    }
    
    if(slot)
    {
        slot->node = node;
        slot->hash = hash;
    }
    
    return !!slot;
}

MD_FUNCTION_IMPL MD_Token
MD_ZeroToken(void)
{
    MD_Token token;
    _MD_MemoryZero(&token, sizeof(token));
    return token;
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsWhitespace(MD_TokenKind kind)
{
    return kind > MD_TokenKind_WhitespaceMin && kind < MD_TokenKind_WhitespaceMax;
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsComment(MD_TokenKind kind)
{
    return(kind == MD_TokenKind_Comment);
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsRegular(MD_TokenKind kind)
{
    return(kind > MD_TokenKind_RegularMin && kind < MD_TokenKind_RegularMax);
}

MD_FUNCTION_IMPL MD_ParseCtx
MD_Parse_InitializeCtx(MD_String8 filename, MD_String8 contents)
{
    MD_ParseCtx ctx = {0};
    ctx.first_root = ctx.last_root = MD_NilNode();
    ctx.at = contents.str;
    ctx.file_contents = contents;
    ctx.filename = filename;
    return ctx;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_PushNodeToList(MD_Node **firstp, MD_Node **lastp, MD_Node *parent, MD_Node *node)
{
    if(!MD_NodeIsNil(node))
    {
        MD_Node *first = *firstp;
        MD_Node *last = *lastp;
        if(MD_NodeIsNil(last))
        {
            first = last = node;
            node->next = node->prev = MD_NilNode();
        }
        else
        {
            last->next = node;
            node->next = MD_NilNode();
            node->prev = last;
            last = last->next;
        }
        *firstp = first;
        *lastp = last;
        node->parent = parent;
    }
}

MD_FUNCTION_IMPL void
MD_Parse_Bump(MD_ParseCtx *ctx, MD_Token token)
{
    ctx->at = token.outer_string.str + token.outer_string.size;
}

MD_FUNCTION_IMPL void
MD_Parse_BumpNext(MD_ParseCtx *ctx)
{
    MD_Parse_Bump(ctx, MD_Parse_LexNext(ctx));
}

MD_FUNCTION_IMPL MD_Token
MD_Parse_LexNext(MD_ParseCtx *ctx)
{
    MD_Token token;
    _MD_MemoryZero(&token, sizeof(token));
    
    MD_u8 *one_past_last = ctx->file_contents.str + ctx->file_contents.size;
    MD_u8 *first = ctx->at;
    if (first < one_past_last)
    {
        MD_u8 *at = first;
        MD_u32 skip_chop_n = 0;
        
#define MD_TokenizerScan(cond) for (; at < one_past_last && (cond); at += 1)
        
        switch (*at)
        {
            // NOTE(allen): Whitespace parsing
            case '\n':
            {
                token.kind = MD_TokenKind_Newline;
                at += 1;
            }break;
            
            case ' ': case '\r': case '\t': case '\f': case '\v':
            {
                token.kind = MD_TokenKind_Whitespace;
                at += 1;
                MD_TokenizerScan(*at == ' ' || *at == '\r' || *at == '\t' || *at == '\f' || *at == '\v');
            }break;
            
            // NOTE(allen): Comment parsing
            case '/':
            {
                if (at + 1 < one_past_last)
                {
                    if (at[1] == '/')
                    {
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        MD_TokenizerScan(*at != '\n');
                    }
                    else if (at[1] == '*')
                    {
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        int counter = 1;
                        for (;at < one_past_last && counter > 0; at += 1)
                        {
                            if (at + 1 < one_past_last)
                            {
                                if (at[0] == '*' && at[1] == '/')
                                {
                                    at += 1;
                                    counter -= 1;
                                }
                                else if (at[0] == '/' && at[1] == '*')
                                {
                                    at += 1;
                                    counter += 1;
                                }
                            }
                        }
                    }
                }
                if (token.kind == MD_TokenKind_Nil) goto symbol_lex;
            }break;
            
            // NOTE(rjf): "Bundle-of-tokens" strings (`stuff` or ```stuff```)
            // In practice no different than a regular string, but provides an
            // alternate syntax which will allow tools like 4coder to treat the
            // contents as regular tokens.
            case '`':
            {
                token.kind = MD_TokenKind_StringLiteral;
                if (at + 2 < one_past_last && at[1] == '`' && at[2] == '`')
                {
                    skip_chop_n = 3;
                    at += 3;
                    MD_TokenizerScan(!(at + 2 < one_past_last && at[0] == '`' && at[1] == '`' && at[2] == '`'));
                    at += 3;
                }
                else
                {
                    // TODO(allen): escape sequences?
                    skip_chop_n = 1;
                    at += 1;
                    MD_TokenizerScan(*at != '\n' && *at != '`');
                    if (*at == '`') at += 1;
                }
            }break;
            
            // NOTE(allen): Strings
            case '"':
            {
                token.kind = MD_TokenKind_StringLiteral;
                if (at + 2 < one_past_last && at[1] == '"' && at[2] == '"')
                {
                    skip_chop_n = 3;
                    at += 3;
                    MD_TokenizerScan(!(at + 2 < one_past_last && at[0] == '"' && at[1] == '"' && at[2] == '"'));
                    at += 3;
                }
                else
                {
                    // TODO(allen): escape sequences?
                    skip_chop_n = 1;
                    at += 1;
                    MD_TokenizerScan(*at != '\n' && *at != '"');
                    if (*at == '"') at += 1;
                }
            }break;
            
            case '\'':
            {
                if (at + 2 < one_past_last && at[1] == '\'' && at[2] == '\'')
                {
                    token.kind = MD_TokenKind_StringLiteral;
                    skip_chop_n = 3;
                    at += 3;
                    MD_TokenizerScan(!(at + 2 < one_past_last && at[0] == '\'' && at[1] == '\'' && at[2] == '\''));
                    at += 3;
                }
                else
                {
                    token.kind = MD_TokenKind_CharLiteral;
                    // TODO(allen): escape sequences?
                    skip_chop_n = 1;
                    at += 1;
                    MD_TokenizerScan(*at != '\n' && *at != '\'');
                    if (*at == '\'') at += 1;
                }
            }break;
            
            // NOTE(allen): Identifiers, Numbers, Operators
            default:
            {
                if (MD_CharIsAlpha(*at))
                {
                    token.kind = MD_TokenKind_Identifier;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '_');
                }
                
                else if (MD_CharIsDigit(*at) ||
                         (at + 1 < one_past_last && at[0] == '-' && MD_CharIsDigit(at[1])))
                {
                    token.kind = MD_TokenKind_NumericLiteral;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '.');
                }
                
                else if (MD_CharIsSymbol(*at))
                {
                    symbol_lex:
                    token.kind = MD_TokenKind_Symbol;
                    at += 1;
                }
            }break;
        }
        
        token.outer_string = MD_S8Range(first, at);
        token.string = MD_StringSubstring(token.outer_string, skip_chop_n, token.outer_string.size - skip_chop_n);
        
        ctx->at = at;
    }
    
    return token;
}

MD_FUNCTION_IMPL MD_Token
MD_Parse_PeekSkipSome(MD_ParseCtx *ctx, MD_TokenGroups skip_groups)
{
    MD_ParseCtx ctx_restore = *ctx;
    
    MD_b32 skip_comment    = (skip_groups & MD_TokenGroup_Comment);
    MD_b32 skip_whitespace = (skip_groups & MD_TokenGroup_Whitespace);
    MD_b32 skip_regular    = (skip_groups & MD_TokenGroup_Regular);
    
    MD_Token result;
    _MD_MemoryZero(&result, sizeof(result));
    
    loop:
    {
        result = MD_Parse_LexNext(ctx);
        if ((skip_comment    && MD_TokenKindIsComment(result.kind)) ||
            (skip_whitespace && MD_TokenKindIsWhitespace(result.kind)) ||
            (skip_regular    && MD_TokenKindIsRegular(result.kind))){
            MD_Parse_Bump(ctx, result);
            goto loop;
        }
    }
    
    {
        // TODO(allen): I'm not a fan of what this implies.
        *ctx = ctx_restore;
    }
    
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_TokenMatch(MD_Token token, MD_String8 string, MD_StringMatchFlags flags)
{
    return MD_StringMatch(token.string, string, flags);
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_Require(MD_ParseCtx *ctx, MD_String8 string)
{
    int result = 0;
    
    MD_Token token_any = MD_Parse_PeekSkipSome(ctx, 0);
    MD_Token token_regular;
    if(MD_StringMatch(token_any.string, string, 0))
    {
        result = 1;
        MD_Parse_Bump(ctx, token_any);
        goto end;
    }
    
    token_regular = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
    if(MD_StringMatch(token_regular.string, string, 0))
    {
        result = 1;
        MD_Parse_Bump(ctx, token_regular);
        goto end;
    }
    
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_RequireKind(MD_ParseCtx *ctx, MD_TokenKind kind, MD_Token *out_token)
{
    int result = 0;
    
    MD_TokenGroups skip_groups = MD_TokenGroup_Comment|MD_TokenGroup_Whitespace;
    if (MD_TokenKindIsWhitespace(kind))
    {
        skip_groups &= ~MD_TokenGroup_Whitespace;
    }
    if (MD_TokenKindIsComment(kind))
    {
        skip_groups &= ~MD_TokenGroup_Comment;
    }
    
    MD_Token token = MD_Parse_PeekSkipSome(ctx, skip_groups);
    if(token.kind == kind)
    {
        result = 1;
        MD_Parse_Bump(ctx, token);
        if(out_token)
        {
            *out_token = token;
        }
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_Error(MD_ParseCtx *ctx, char *fmt, ...)
{
    MD_Error *error = _MD_PushArray(_MD_GetCtx(), MD_Error, 1);
    error->filename = ctx->filename;
    va_list args;
    va_start(args, fmt);
    error->string = MD_PushStringFV(fmt, args);
    va_end(args);
}

MD_PRIVATE_FUNCTION_IMPL MD_Node *
_MD_MakeNode(MD_NodeKind kind, MD_String8 string, MD_String8 whole_string, MD_String8 filename,
             MD_u8 *file_contents, MD_u8 *at)
{
    MD_Node *node = _MD_PushArray(_MD_GetCtx(), MD_Node, 1);
    if(node)
    {
        node->kind = kind;
        node->string = string;
        node->whole_string = whole_string;
        node->next = node->prev = node->parent = node->first_child = node->last_child = node->first_tag = node->last_tag = MD_NilNode();
        node->filename = filename;
        node->file_contents = file_contents;
        node->at = at;
    }
    else
    {
        node = MD_NilNode();
    }
    return node;
}

MD_PRIVATE_FUNCTION_IMPL MD_Node *
_MD_MakeNodeFromToken_Ctx(MD_ParseCtx *ctx, MD_NodeKind kind, MD_Token token)
{
    return _MD_MakeNode(kind, token.string, token.outer_string, ctx->filename, ctx->file_contents.str, ctx->at);
}

MD_PRIVATE_FUNCTION_IMPL MD_Node *
_MD_MakeNodeFromString_Ctx(MD_ParseCtx *ctx, MD_NodeKind kind, MD_String8 string)
{
    return _MD_MakeNode(kind, string, string, ctx->filename, ctx->file_contents.str, ctx->at);
}

typedef MD_u32 _MD_ParseSetFlags;
enum
{
    _MD_ParseSetFlag_Paren    = (1<<0),
    _MD_ParseSetFlag_Brace    = (1<<1),
    _MD_ParseSetFlag_Bracket  = (1<<2),
    _MD_ParseSetFlag_Implicit = (1<<3),
};

MD_PRIVATE_FUNCTION_IMPL MD_ParseResult _MD_ParseOneNode(MD_ParseCtx *ctx);
MD_PRIVATE_FUNCTION_IMPL void _MD_ParseSet(MD_ParseCtx *ctx, MD_Node *parent, _MD_ParseSetFlags flags, MD_Node **first_out, MD_Node **last_out);
MD_PRIVATE_FUNCTION_IMPL void _MD_ParseTagList(MD_ParseCtx *ctx, MD_Node **first_out, MD_Node **last_out);

MD_PRIVATE_FUNCTION_IMPL void
_MD_SetNodeFlagsByToken(MD_Node *node, MD_Token token)
{
#define Flag(_kind, _flag) if(token.kind == _kind) { node->flags |= _flag; }
    Flag(MD_TokenKind_Identifier,     MD_NodeFlag_Identifier);
    Flag(MD_TokenKind_NumericLiteral, MD_NodeFlag_Numeric);
    Flag(MD_TokenKind_StringLiteral,  MD_NodeFlag_StringLiteral);
    Flag(MD_TokenKind_CharLiteral,    MD_NodeFlag_CharLiteral);
#undef Flag
}

MD_PRIVATE_FUNCTION_IMPL MD_ParseResult
_MD_ParseOneNode(MD_ParseCtx *ctx)
{
    MD_u8 *at_first = ctx->at;
    
    MD_ParseResult result = {0};
    result.node = MD_NilNode();
    
    MD_Token token;
    _MD_MemoryZero(&token, sizeof(token));
    
    MD_Node *first_tag = 0;
    MD_Node *last_tag = 0;
    _MD_ParseTagList(ctx, &first_tag, &last_tag);
    
    // NOTE(rjf): Unnamed Sets
    MD_TokenGroups skip_groups = MD_TokenGroup_Whitespace|MD_TokenGroup_Comment;
    if(MD_Parse_TokenMatch(MD_Parse_PeekSkipSome(ctx, skip_groups), MD_S8Lit("("), 0) ||
       MD_Parse_TokenMatch(MD_Parse_PeekSkipSome(ctx, skip_groups), MD_S8Lit("{"), 0) ||
       MD_Parse_TokenMatch(MD_Parse_PeekSkipSome(ctx, skip_groups), MD_S8Lit("["), 0))
    {
        result.node = _MD_MakeNodeFromString_Ctx(ctx, MD_NodeKind_UnnamedSet, MD_S8Lit(""));
        _MD_ParseSet(ctx, result.node,
                     _MD_ParseSetFlag_Paren   |
                     _MD_ParseSetFlag_Brace   |
                     _MD_ParseSetFlag_Bracket,
                     &result.node->first_child,
                     &result.node->last_child);
        goto end_parse;
    }
    
    // NOTE(rjf): Labels
    else if(MD_Parse_RequireKind(ctx, MD_TokenKind_Identifier,     &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_NumericLiteral, &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_StringLiteral,  &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_CharLiteral,    &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_Symbol,         &token))
    {
        result.node = MD_MakeNodeFromToken(MD_NodeKind_Label, ctx->filename, ctx->file_contents.str, ctx->at, token);
        _MD_SetNodeFlagsByToken(result.node, token);
        // NOTE(rjf): Children
        if(MD_Parse_Require(ctx, MD_S8Lit(":")))
        {
            _MD_ParseSet(ctx, result.node,
                         _MD_ParseSetFlag_Paren   |
                         _MD_ParseSetFlag_Brace   |
                         _MD_ParseSetFlag_Bracket |
                         _MD_ParseSetFlag_Implicit,
                         &result.node->first_child,
                         &result.node->last_child);
        }
        goto end_parse;
    }
    
    end_parse:;
    if(!MD_NodeIsNil(result.node))
    {
        result.bytes_parsed = (MD_u64)(ctx->at - at_first);
        result.node->first_tag = first_tag;
        result.node->last_tag = last_tag;
        for(MD_Node *tag = first_tag; !MD_NodeIsNil(tag); tag = tag->next)
        {
            tag->parent = result.node;
        }
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_ParseSet(MD_ParseCtx *ctx, MD_Node *parent, _MD_ParseSetFlags flags,
             MD_Node **first_out, MD_Node **last_out)
{
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    
    MD_b32 brace = 0;
    MD_b32 paren = 0;
    MD_b32 bracket = 0;
    MD_b32 terminate_with_separator = !!(flags & _MD_ParseSetFlag_Implicit);
    
    if(flags & _MD_ParseSetFlag_Brace && MD_Parse_Require(ctx, MD_S8Lit("{")))
    {
        parent->flags |= MD_NodeFlag_BraceLeft;
        brace = 1;
        terminate_with_separator = 0;
    }
    else if(flags & _MD_ParseSetFlag_Paren && MD_Parse_Require(ctx, MD_S8Lit("(")))
    {
        parent->flags |= MD_NodeFlag_ParenLeft;
        paren = 1;
        terminate_with_separator = 0;
    }
    else if(flags & _MD_ParseSetFlag_Bracket && MD_Parse_Require(ctx, MD_S8Lit("[")))
    {
        parent->flags |= MD_NodeFlag_BracketLeft;
        bracket = 1;
        terminate_with_separator = 0;
    }
    
    // NOTE(rjf): Parse children.
    if(brace || paren || bracket || terminate_with_separator)
    {
        MD_NodeFlags next_child_flags = 0;
        for(MD_u64 child_idx = 0;; child_idx += 1)
        {
            if(brace)
            {
                if(MD_Parse_Require(ctx, MD_S8Lit("}")))
                {
                    parent->flags |= MD_NodeFlag_BraceRight;
                    goto end_parse;
                }
            }
            else if(paren || bracket)
            {
                if(flags & _MD_ParseSetFlag_Paren && MD_Parse_Require(ctx, MD_S8Lit(")")))
                {
                    parent->flags |= MD_NodeFlag_ParenRight;
                    goto end_parse;
                }
                else if(flags & _MD_ParseSetFlag_Bracket && MD_Parse_Require(ctx, MD_S8Lit("]")))
                {
                    parent->flags |= MD_NodeFlag_BracketRight;
                    goto end_parse;
                }
            }
            else
            {
                MD_Token peek = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Whitespace | MD_TokenGroup_Comment);
                if(MD_Parse_TokenMatch(peek, MD_S8Lit("}"), 0) ||
                   MD_Parse_TokenMatch(peek, MD_S8Lit(")"), 0) ||
                   MD_Parse_TokenMatch(peek, MD_S8Lit("]"), 0))
                {
                    goto end_parse;
                }
            }
            
            MD_ParseResult parse = _MD_ParseOneNode(ctx);
            MD_Node *child = parse.node;
            child->flags |= next_child_flags;
            next_child_flags = 0;
            if(MD_NodeIsNil(child))
            {
                goto end_parse;
            }
            else
            {
                _MD_PushNodeToList(&first, &last, parent, child);
            }
            
            // NOTE(rjf): Separators.
            {
                MD_b32 result = 0;
                if(MD_Parse_Require(ctx, MD_S8Lit(",")))
                {
                    result |= 1;
                    child->flags |= MD_NodeFlag_BeforeComma;
                    next_child_flags |= MD_NodeFlag_AfterComma;
                }
                else if(MD_Parse_Require(ctx, MD_S8Lit(";")))
                {
                    result |= 1;
                    child->flags |= MD_NodeFlag_BeforeSemicolon;
                    next_child_flags |= MD_NodeFlag_AfterSemicolon;
                }
                result |= !!MD_Parse_Require(ctx, MD_S8Lit("\n"));
                if(result && terminate_with_separator)
                {
                    goto end_parse;
                }
            }
        }
    }
    
    end_parse:;
    *first_out = first;
    *last_out = last;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_ParseTagList(MD_ParseCtx *ctx, MD_Node **first_out, MD_Node **last_out)
{
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    
    for(;MD_Parse_Require(ctx, MD_S8Lit("@"));)
    {
        MD_Token name;
        _MD_MemoryZero(&name, sizeof(name));
        if(MD_Parse_RequireKind(ctx, MD_TokenKind_Identifier, &name))
        {
            MD_Node *tag = _MD_MakeNodeFromToken_Ctx(ctx, MD_NodeKind_Tag, name);
            _MD_ParseSet(ctx, tag, _MD_ParseSetFlag_Paren, &tag->first_child, &tag->last_child);
            _MD_PushNodeToList(&first, &last, MD_NilNode(), tag);
        }
        else
        {
            break;
        }
    }
    
    *first_out = first;
    *last_out = last;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseOneNode(MD_String8 filename, MD_String8 contents)
{
    MD_ParseCtx ctx = MD_Parse_InitializeCtx(filename, contents);
    return _MD_ParseOneNode(&ctx);
}

MD_FUNCTION MD_Node *
MD_ParseWholeString(MD_String8 filename, MD_String8 contents)
{
    MD_Node *root = MD_MakeNodeFromString(MD_NodeKind_File, filename, contents.str, contents.str, MD_PushStringF("`DD Parsed From \"%.*s\"`", MD_StringExpand(filename)));
    if(contents.size > 0)
    {
        MD_ParseCtx ctx = MD_Parse_InitializeCtx(filename, contents);
        for(MD_u64 i = 0; i < contents.size;)
        {
            MD_ParseResult parse = _MD_ParseOneNode(&ctx);
            _MD_PushNodeToList(&root->first_child, &root->last_child, root, parse.node);
            i += parse.bytes_parsed;
            if(parse.bytes_parsed == 0)
            {
                break;
            }
        }
    }
    return root;
}

MD_FUNCTION_IMPL MD_Node *
MD_ParseWholeFile(MD_String8 filename)
{
    return MD_ParseWholeString(filename, MD_LoadEntireFile(filename));
}

MD_FUNCTION_IMPL MD_b32
MD_NodeIsNil(MD_Node *node)
{
    return node == 0 || node == &_md_nil_node || node->kind == MD_NodeKind_Nil;
}

MD_FUNCTION_IMPL MD_Node *
MD_NilNode(void) { return &_md_nil_node; }

MD_FUNCTION_IMPL MD_Node *
MD_MakeNodeFromToken(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_Token token)
{
    return _MD_MakeNode(kind, token.string, token.outer_string, filename, file, at);
}

MD_FUNCTION_IMPL MD_Node *
MD_MakeNodeFromString(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_String8 string)
{
    return _MD_MakeNode(kind, string, string, filename, file, at);
}

MD_FUNCTION_IMPL void
MD_PushSibling(MD_Node **first, MD_Node **last, MD_Node *parent, MD_Node *new_sibling)
{
    _MD_PushNodeToList(first, last, parent, new_sibling);
}

MD_FUNCTION_IMPL void
MD_PushChild(MD_Node *parent, MD_Node *new_child)
{
    _MD_PushNodeToList(&parent->first_child, &parent->last_child, parent, new_child);
}

MD_FUNCTION_IMPL void
MD_PushTag(MD_Node *node, MD_Node *tag)
{
    _MD_PushNodeToList(&node->first_tag, &node->last_tag, node, tag);
}

MD_FUNCTION_IMPL MD_b32
MD_NodeMatch(MD_Node *a, MD_Node *b, MD_StringMatchFlags str_flags, MD_NodeMatchFlags node_flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind && MD_StringMatch(a->string, b->string, str_flags))
    {
        result = 1;
        if(a->kind != MD_NodeKind_Tag && node_flags & MD_NodeMatchFlag_Tags)
        {
            for(MD_Node *a_tag = a->first_tag, *b_tag = b->first_tag;
                !MD_NodeIsNil(a_tag) || !MD_NodeIsNil(b_tag);
                a_tag = a_tag->next, b_tag = b_tag->next)
            {
                if(MD_NodeMatch(a_tag, b_tag, str_flags, 0))
                {
                    if(node_flags & MD_NodeMatchFlag_TagArguments)
                    {
                        for(MD_Node *a_tag_arg = a_tag->first_child, *b_tag_arg = b_tag->first_child;
                            !MD_NodeIsNil(a_tag_arg) || !MD_NodeIsNil(b_tag_arg);
                            a_tag_arg = a_tag_arg->next, b_tag_arg = b_tag_arg->next)
                        {
                            if(!MD_NodeDeepMatch(a_tag_arg, b_tag_arg, str_flags, node_flags))
                            {
                                result = 0;
                                goto end;
                            }
                        }
                    }
                }
                else
                {
                    result = 0;
                    goto end;
                }
            }
        }
    }
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_StringMatchFlags str_flags, MD_NodeMatchFlags node_flags)
{
    MD_b32 result = MD_NodeMatch(a, b, str_flags, node_flags);
    if(result)
    {
        for(MD_Node *a_child = a->first_child, *b_child = b->first_child;
            !MD_NodeIsNil(a_child) || !MD_NodeIsNil(b_child);
            a_child = a_child->next, b_child = b_child->next)
        {
            if(!MD_NodeDeepMatch(a_child, b_child, str_flags, node_flags))
            {
                result = 0;
                goto end;
            }
        }
    }
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromString(MD_Node *first, MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next)
    {
        if(MD_StringMatch(string, node->string, 0))
        {
            result = node;
            break;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromIndex(MD_Node *first, MD_Node *last, int n)
{
    MD_Node *result = MD_NilNode();
    if(n >= 0)
    {
        int idx = 0;
        for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next, idx += 1)
        {
            if(idx == n)
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL int
MD_IndexFromNode(MD_Node *node)
{
    int idx = 0;
    if(node && !MD_NodeIsNil(node))
    {
        for(MD_Node *last = node->prev; !MD_NodeIsNil(last); last = last->prev, idx += 1);
    }
    return idx;
}

MD_FUNCTION_IMPL MD_Node *
MD_NextNodeSibling(MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    if(last)
    {
        for(MD_Node *node = last->next; node; node = node->next)
        {
            if(MD_StringMatch(string, node->string, 0))
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromString(MD_Node *node, MD_String8 child_string)
{
    return MD_NodeFromString(node->first_child, node->last_child, child_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromString(MD_Node *node, MD_String8 tag_string)
{
    return MD_NodeFromString(node->first_tag, node->last_tag, tag_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, int n)
{
    MD_Node *tag = MD_TagFromString(node, tag_string);
    return MD_ChildFromIndex(tag, n);
}

MD_FUNCTION_IMPL MD_b32
MD_NodeHasTag(MD_Node *node, MD_String8 tag_string)
{
    return !MD_NodeIsNil(MD_TagFromString(node, tag_string));
}

MD_FUNCTION_IMPL MD_CodeLoc
MD_CodeLocFromNode(MD_Node *node)
{
    MD_CodeLoc loc;
    loc.filename = node->filename;
    loc.line = 1;
    loc.column = 1;
    for(MD_u64 i = 0; node->file_contents[i]; i += 1)
    {
        if(node->file_contents[i] == '\n')
        {
            loc.line += 1;
            loc.column = 1;
        }
        else
        {
            loc.column += 1;
        }
        if(node->file_contents + i == node->at)
        {
            break;
        }
    }
    return loc;
}

MD_FUNCTION_IMPL MD_i64
MD_ChildCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_i64
MD_TagCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(tag, node->first_tag))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_i64
MD_ChildCountFromNodeAndString(MD_Node *node, MD_String8 string, MD_StringMatchFlags flags)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        if(MD_StringMatch(child->string, string, flags))
        {
            result += 1;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_i64
MD_TagCountFromNodeAndString(MD_Node *node, MD_String8 string, MD_StringMatchFlags flags)
{
    MD_i64 result = 0;
    for(MD_EachNode(tag, node->first_tag))
    {
        if(MD_StringMatch(tag->string, string, flags))
        {
            result += 1;
        }
    }
    return result;
}

MD_FUNCTION_IMPL void
MD_NodeMessage(MD_Node *node, MD_MessageKind kind, MD_String8 str)
{
    const char *kind_name = kind == MD_MessageKind_Error ? "error" : "warning";
    MD_CodeLoc loc = MD_CodeLocFromNode(node);
    fprintf(stderr, "%.*s:%i:%i: %s: %.*s\n",
            MD_StringExpand(loc.filename),
            loc.line, loc.column,
            kind_name,
            MD_StringExpand(str));
}

MD_FUNCTION_IMPL void
MD_NodeError(MD_Node *node, MD_String8 str)
{
    MD_NodeMessage(node, MD_MessageKind_Error, str);
}

MD_FUNCTION_IMPL void
MD_NodeWarning(MD_Node *node, MD_String8 str)
{
    MD_NodeMessage(node, MD_MessageKind_Warning, str);
}

MD_FUNCTION_IMPL void
MD_NodeMessageF(MD_Node *node, MD_MessageKind kind, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_NodeMessage(node, kind ,MD_PushStringFV(fmt, args));
    va_end(args);
}

MD_FUNCTION_IMPL void
MD_NodeErrorF(MD_Node *node, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_NodeError(node, MD_PushStringFV(fmt, args));
    va_end(args);
}

MD_FUNCTION_IMPL void
MD_NodeWarningF(MD_Node *node, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_NodeWarning(node, MD_PushStringFV(fmt, args));
    va_end(args);
}

MD_GLOBAL MD_Expr _md_nil_expr =
{
    &_md_nil_node,
    MD_ExprKind_Nil,
    &_md_nil_expr,
    {&_md_nil_expr, &_md_nil_expr },
};

MD_FUNCTION_IMPL MD_Expr *
MD_NilExpr(void)
{
    return &_md_nil_expr;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprIsNil(MD_Expr *expr)
{
    return expr == 0 || expr == &_md_nil_expr || expr->kind == MD_ExprKind_Nil;
}

typedef enum _MD_ExprKindGroup
{
    _MD_ExprKindGroup_Nil,
    _MD_ExprKindGroup_Atom,
    _MD_ExprKindGroup_Binary,
    _MD_ExprKindGroup_PreUnary,
    _MD_ExprKindGroup_PostUnary,
    _MD_ExprKindGroup_Type,
}
_MD_ExprKindGroup;

typedef struct _MD_ExprKindMetadata _MD_ExprKindMetadata;
struct _MD_ExprKindMetadata
{
    _MD_ExprKindGroup group;
    MD_ExprPrec prec;
    char *symbol;
    char *pre_symbol;
    char *post_symbol;
};

MD_FUNCTION_IMPL _MD_ExprKindMetadata *
_MD_MetadataFromExprKind(MD_ExprKind kind)
{
    // 0:  Invalid
    // 12: (unary) - ~ !
    // 11: . -> () []
    // 10: * / %
    // 9:  + -
    // 8:  << >>
    // 7:  < <= > >=
    // 6:  == !=
    // 5:  (bitwise) &
    // 4:  ^
    // 3:  |
    // 2:  &&
    // 1:  ||
    static _MD_ExprKindMetadata metadata[] =
    {
        {_MD_ExprKindGroup_Nil,       +0, "NIL",   "",  "" },  // MD_ExprKind_Nil
        {_MD_ExprKindGroup_Atom,      +0, "NIL",   "",  "" },  // MD_ExprKind_Atom
        {_MD_ExprKindGroup_Binary,    +11, ".",     "",  "" },  // MD_ExprKind_Dot
        {_MD_ExprKindGroup_Binary,    +11, "->",    "",  "" },  // MD_ExprKind_Arrow
        {_MD_ExprKindGroup_PostUnary, +11, "",      "(", ")"},  // MD_ExprKind_Call
        {_MD_ExprKindGroup_PostUnary, +11, "",      "[", "]"},  // MD_ExprKind_Subscript
        {_MD_ExprKindGroup_PreUnary,  +12, "",      "*", "" },  // MD_ExprKind_Dereference
        {_MD_ExprKindGroup_PreUnary,  +12, "",      "&", "" },  // MD_ExprKind_Reference
        {_MD_ExprKindGroup_Binary,    +9,  "+",     "",  "" },  // MD_ExprKind_Add
        {_MD_ExprKindGroup_Binary,    +9,  "-",     "",  "" },  // MD_ExprKind_Subtract
        {_MD_ExprKindGroup_Binary,    +10, "*",     "",  "" },  // MD_ExprKind_Multiply
        {_MD_ExprKindGroup_Binary,    +10, "/",     "",  "" },  // MD_ExprKind_Divide
        {_MD_ExprKindGroup_Binary,    +10, "%",     "",  "" },  // MD_ExprKind_Mod
        {_MD_ExprKindGroup_Binary,    +6,  "==",    "",  "" },  // MD_ExprKind_IsEqual
        {_MD_ExprKindGroup_Binary,    +6,  "!=",    "",  "" },  // MD_ExprKind_IsNotEqual
        {_MD_ExprKindGroup_Binary,    +7,  "<",     "",  "" },  // MD_ExprKind_LessThan
        {_MD_ExprKindGroup_Binary,    +7,  ">",     "",  "" },  // MD_ExprKind_GreaterThan
        {_MD_ExprKindGroup_Binary,    +7,  "<=",    "",  "" },  // MD_ExprKind_LessThanEqualTo
        {_MD_ExprKindGroup_Binary,    +7,  ">=",    "",  "" },  // MD_ExprKind_GreaterThanEqualTo
        {_MD_ExprKindGroup_Binary,    +2,  "&&",    "",  "" },  // MD_ExprKind_BoolAnd
        {_MD_ExprKindGroup_Binary,    +1,  "||",    "",  "" },  // MD_ExprKind_BoolOr
        {_MD_ExprKindGroup_PreUnary,  +12, "",      "!", "" },  // MD_ExprKind_BoolNot
        {_MD_ExprKindGroup_Binary,    +5,  "&",     "",  "" },  // MD_ExprKind_BitAnd
        {_MD_ExprKindGroup_Binary,    +3,  "|",     "",  "" },  // MD_ExprKind_BitOr
        {_MD_ExprKindGroup_PreUnary,  +12, "",      "~", "" },  // MD_ExprKind_BitNot
        {_MD_ExprKindGroup_Binary,    +4,  "^",     "",  "" },  // MD_ExprKind_BitXor
        {_MD_ExprKindGroup_Binary,    +8,  "<<",    "",  "" },  // MD_ExprKind_LeftShift
        {_MD_ExprKindGroup_Binary,    +8,  ">>",    "",  "" },  // MD_ExprKind_RightShift
        {_MD_ExprKindGroup_PreUnary,  +12, "",      "-", "" },  // MD_ExprKind_Negative
        {_MD_ExprKindGroup_Type,      +0,  "*",     "",  "" },  // MD_ExprKind_Pointer
        {_MD_ExprKindGroup_Type,      +0,  "",      "[", "]"},  // MD_ExprKind_Array
    };
    return &metadata[kind];
}

MD_FUNCTION_IMPL MD_ExprKind
MD_PreUnaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    // NOTE(rjf): Special-cases for calls/subscripts.
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_ExprKind_Subscript;
        }
    }
    else
    {
        for(MD_ExprKind kind_it = (MD_ExprKind)0; kind_it < MD_ExprKind_MAX;
            kind_it = (MD_ExprKind)((int)kind_it + 1))
        {
            _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == _MD_ExprKindGroup_PreUnary)
            {
                if(MD_StringMatch(node->string, MD_S8CString(metadata->pre_symbol), 0))
                {
                    kind = kind_it;
                    break;
                }
            }
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_ExprKind
MD_PostUnaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_ExprKind_Subscript;
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_ExprKind
MD_BinaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    if(node->kind == MD_NodeKind_Label && MD_NodeIsNil(node->first_child))
    {
        for(MD_ExprKind kind_it = (MD_ExprKind)0; kind_it < MD_ExprKind_MAX;
            kind_it = (MD_ExprKind)((int)kind_it + 1))
        {
            _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == _MD_ExprKindGroup_Binary)
            {
                if(MD_StringMatch(node->string, MD_S8CString(metadata->symbol), 0))
                {
                    kind = kind_it;
                    break;
                }
            }
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_ExprPrec
MD_ExprPrecFromExprKind(MD_ExprKind kind)
{
    _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind);
    return metadata->prec;
}

MD_FUNCTION_IMPL MD_Expr *
MD_MakeExpr(MD_Node *node, MD_ExprKind kind, MD_Expr *left, MD_Expr *right)
{
    MD_Expr *expr = _MD_PushArray(_MD_GetCtx(), MD_Expr, 1);
    if(expr)
    {
        if(left == 0)  left  = MD_NilExpr();
        if(right == 0) right = MD_NilExpr();
        expr->node = node;
        expr->kind = kind;
        expr->sub[0] = left;
        expr->sub[1] = right;
    }
    else
    {
        expr = MD_NilExpr();
    }
    return expr;
}

typedef struct _MD_NodeParseCtx _MD_NodeParseCtx;
struct _MD_NodeParseCtx
{
    MD_Node *at;
    MD_Node *last;
    MD_Node *one_past_last;
};

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeAtom(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(ctx->at->kind == MD_NodeKind_Label &&
       MD_NodeIsNil(ctx->at->first_child))
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeSet(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(!MD_NodeIsNil(ctx->at->first_child))
    {
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
        result = 1;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeLiteral(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(ctx->at->flags & MD_NodeFlag_StringLiteral ||
       ctx->at->flags & MD_NodeFlag_CharLiteral   ||
       ctx->at->flags & MD_NodeFlag_Numeric)
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_Consume(_MD_NodeParseCtx *ctx, MD_String8 string, MD_Node **out)
{
    MD_b32 result = 0;
    if(MD_StringMatch(ctx->at->string, string, 0))
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_NodeParse_Next(_MD_NodeParseCtx *ctx)
{
    ctx->at = ctx->at->next;
}

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in);

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx);

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseUnaryExpr(_MD_NodeParseCtx *ctx)
{
    MD_Expr *result = MD_NilExpr();
    MD_Node *set = 0;
    MD_Node *node = 0;
    
    // NOTE(rjf): Sub-expression
    if(_MD_NodeParse_ConsumeSet(ctx, &set))
    {
        result = MD_ParseAsExpr(set->first_child, set->last_child);
    }
    
    // NOTE(rjf): Literal
    else if(_MD_NodeParse_ConsumeLiteral(ctx, &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_Atom, 0, 0);
    }
    
    // NOTE(rjf): Negative
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("-"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_Negative, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Bitwise Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("~"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_BitNot, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Boolean Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("!"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_BoolNot, 0, _MD_ParseExpr(ctx));
    }
    
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in)
{
    MD_Expr *expr = _MD_ParseUnaryExpr(ctx);
    MD_ExprKind expr_kind;
    if(MD_ExprIsNil(expr))
    {
        goto end_parse;
    }
    
    expr_kind = MD_BinaryExprKindFromNode(ctx->at);
    if(expr_kind != MD_ExprKind_Nil)
    {
        for(int precedence = MD_ExprPrecFromExprKind(expr_kind);
            precedence >= precedence_in;
            precedence -= 1)
        {
            for(;;)
            {
                MD_Node *op_node = ctx->at;
                expr_kind = MD_BinaryExprKindFromNode(ctx->at);
                int operator_precedence = MD_ExprPrecFromExprKind(expr_kind);
                if(operator_precedence != precedence)
                {
                    break;
                }
                
                if(expr_kind == MD_ExprKind_Nil)
                {
                    break;
                }
                
                _MD_NodeParse_Next(ctx);
                
                MD_Expr *right = _MD_ParseExpr_(ctx, precedence+1);
                if(MD_ExprIsNil(right))
                {
                    // TODO(rjf): Error: "Expected right-hand-side of binary expression."
                    goto end_parse;
                }
                
                MD_Expr *left = expr;
                expr = MD_MakeExpr(op_node, expr_kind, left, right);
                expr->sub[0] = left;
                expr->sub[1] = right;
            }
        }
    }
    
    end_parse:;
    return expr;
}

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx)
{
    return _MD_ParseExpr_(ctx, 1);
}

MD_FUNCTION_IMPL MD_Expr *
MD_ParseAsExpr(MD_Node *first, MD_Node *last)
{
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
    return _MD_ParseExpr(ctx);
}

MD_FUNCTION_IMPL MD_Expr *
MD_ParseAsType(MD_Node *first, MD_Node *last)
{
    MD_Expr *expr = MD_NilExpr();
    MD_Expr *last_expr = expr;
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
#define _MD_PushType(x) if(MD_ExprIsNil(last_expr)) { expr = last_expr = x; } else { last_expr = last_expr->sub[0] = x; }
    MD_Node *set = 0;
    MD_Node *ptr = 0;
    MD_Node *vol = 0;
    MD_Node *base_type = 0;
    for(;;)
    {
        if(_MD_NodeParse_Consume(ctx, MD_S8Lit("*"), &ptr))
        {
            MD_Expr *t = MD_MakeExpr(ptr, MD_ExprKind_Pointer, MD_NilExpr(), MD_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeSet(ctx, &set))
        {
            MD_Expr *t = MD_MakeExpr(set, MD_ExprKind_Array, MD_NilExpr(), MD_NilExpr());
            t->sub[1] = MD_ParseAsExpr(set->first_child, set->last_child);
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeAtom(ctx, &base_type))
        {
            MD_Expr *t = MD_MakeExpr(base_type, MD_ExprKind_Atom, MD_NilExpr(), MD_NilExpr());
            _MD_PushType(t);
        }
        else
        {
            break;
        }
    }
#undef _MD_PushType
    return expr;
}

MD_FUNCTION_IMPL MD_i64
MD_EvaluateExpr_I64(MD_Expr *expr)
{
    MD_i64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_ExprKind_##name: { result = MD_EvaluateExpr_I64(expr->sub[0]) op MD_EvaluateExpr_I64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_ExprKind_Atom: { result = MD_I64FromString(expr->node->string); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_f64
MD_EvaluateExpr_F64(MD_Expr *expr)
{
    MD_f64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_ExprKind_##name: { result = MD_EvaluateExpr_I64(expr->sub[0]) op MD_EvaluateExpr_F64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_ExprKind_Atom: { result = MD_F64FromString(expr->node->string); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprMatch(MD_Expr *a, MD_Expr *b, MD_StringMatchFlags str_flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind)
    {
        result = 1;
        if(a->kind == MD_ExprKind_Atom)
        {
            result = MD_StringMatch(a->node->string, b->node->string, str_flags);
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprDeepMatch(MD_Expr *a, MD_Expr *b, MD_StringMatchFlags str_flags)
{
    MD_b32 result = MD_ExprMatch(a, b, str_flags);
    if(result && !MD_ExprIsNil(a) && !MD_ExprIsNil(b))
    {
        result = (MD_ExprDeepMatch(a->sub[0], b->sub[0], str_flags) &&
                  MD_ExprDeepMatch(a->sub[1], b->sub[1], str_flags));
    }
    return result;
}

MD_FUNCTION_IMPL void
MD_OutputTree(FILE *file, MD_Node *node)
{
    for(MD_Node *tag = node->first_tag; !MD_NodeIsNil(tag); tag = tag->next)
    {
        fprintf(file, "@%.*s", MD_StringExpand(tag->string));
        if(!MD_NodeIsNil(tag->first_child))
        {
            fprintf(file, "(");
            for(MD_Node *child = tag->first_child; !MD_NodeIsNil(child); child = child->next)
            {
                MD_OutputTree(file, child);
                fprintf(file, ", ");
            }
            fprintf(file, ")\n");
        }
        else
        {
            fprintf(file, " ");
        }
    }
    
    fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
    if(!MD_NodeIsNil(node->first_child))
    {
        fprintf(file, ":\n{\n");
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_OutputTree(file, child);
            fprintf(file, ",\n");
        }
        fprintf(file, "}\n");
    }
    else
    {
        fprintf(file, " ");
    }
}

MD_FUNCTION_IMPL void
MD_OutputTree_C_String(FILE *file, MD_Node *node)
{
    fprintf(file, "\"");
    for(MD_u64 i = 0; i < node->string.size; i += 1)
    {
        if(node->string.str[i] == '\n')
        {
            fprintf(file, "\\n\"\n\"");
        }
        else if(node->string.str[i] == '\r' &&
                i+1 < node->string.size && node->string.str[i+1] == '\n')
        {
            // NOTE(mal): Step over CR when quoting CRLF newlines
        }
        else
        {
            fprintf(file, "%c", node->string.str[i]);
        }
    }
    fprintf(file, "\"");
}

MD_FUNCTION_IMPL void
MD_OutputTree_C_Struct(FILE *file, MD_Node *node)
{
    if(node)
    {
        fprintf(file, "typedef struct %.*s %.*s;\n",
                MD_StringExpand(node->string),
                MD_StringExpand(node->string));
        fprintf(file, "struct %.*s\n{\n", MD_StringExpand(node->string));
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_OutputTree_C_Decl(file, child);
            fprintf(file, ";\n");
        }
        fprintf(file, "};\n\n");
    }
}

MD_FUNCTION_IMPL void
MD_OutputTree_C_Decl(FILE *file, MD_Node *node)
{
    if(node)
    {
        MD_Expr *type = MD_ParseAsType(node->first_child, node->last_child);
        MD_Output_C_DeclByNameAndType(file, node->string, type);
    }
}

MD_FUNCTION_IMPL void
MD_Output_C_DeclByNameAndType(FILE *file, MD_String8 name, MD_Expr *type)
{
    MD_OutputType_C_LHS(file, type);
    fprintf(file, " %.*s", MD_StringExpand(name));
    MD_OutputType_C_RHS(file, type);
}

MD_FUNCTION_IMPL void
MD_OutputExpr(FILE *file, MD_Expr *expr)
{
    if(!MD_NodeIsNil(expr->node))
    {
        _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(expr->kind);
        switch(metadata->group)
        {
            case _MD_ExprKindGroup_Atom:
            {
                fprintf(file, "%.*s", MD_StringExpand(expr->node->string));
            }break;
            
            case _MD_ExprKindGroup_Binary:
            {
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, " %s ", metadata->symbol);
                MD_OutputExpr(file, expr->sub[1]);
                fprintf(file, ")");
            }break;
            
            case _MD_ExprKindGroup_PreUnary:
            {
                fprintf(file, "%s", metadata->pre_symbol);
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, ")");
            }break;
            
            case _MD_ExprKindGroup_PostUnary:
            {
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, ")");
                fprintf(file, "%s", metadata->post_symbol);
            }break;
            
            default: break;
        }
    }
}

MD_FUNCTION_IMPL void
MD_OutputExpr_C(FILE *file, MD_Expr *expr)
{
    MD_OutputExpr(file, expr);
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_OutputType_C_NeedsParens(MD_Expr *type)
{
    MD_b32 result = 0;
    if (type->kind == MD_ExprKind_Pointer &&
        type->sub[0]->kind == MD_ExprKind_Array)
    {
        result = 1;
    }
    return(result);
}

MD_FUNCTION_IMPL void
MD_OutputType_C_LHS(FILE *file, MD_Expr *type)
{
    switch (type->kind)
    {
        case MD_ExprKind_Atom:
        {
            MD_Node *node = type->node;
            fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
        }break;
        
        case MD_ExprKind_Pointer:
        {
            MD_OutputType_C_LHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
            fprintf(file, "*");
        }break;
        
        case MD_ExprKind_Array:
        {
            MD_OutputType_C_LHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
        }break;
        
        default:
        {
            fprintf(file, "{ unexpected MD_ExprKind (%i) in type info for node \"%.*s\" }",
                    type->kind,
                    MD_StringExpand(type->node->whole_string));
        }break;
    }
}

MD_FUNCTION_IMPL void
MD_OutputType_C_RHS(FILE *file, MD_Expr *type)
{
    switch (type->kind)
    {
        case MD_ExprKind_Atom:
        {}break;
        
        case MD_ExprKind_Pointer:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            MD_OutputType_C_RHS(file, type->sub[0]);
        }break;
        
        case MD_ExprKind_Array:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            fprintf(file, "[");
            MD_OutputExpr_C(file, type->sub[1]);
            fprintf(file, "]");
            MD_OutputType_C_RHS(file, type->sub[0]);
        }break;
        
        default:
        {}break;
    }
}

MD_FUNCTION_IMPL MD_CommandLine
MD_CommandLine_Start(int argument_count, char **arguments)
{
    MD_CommandLine cmdln = {0};
    cmdln.arguments = _MD_PushArray(_MD_GetCtx(), MD_String8, argument_count-1);
    for(int i = 1; i < argument_count; i += 1)
    {
        cmdln.arguments[i-1] = MD_PushStringF("%s", arguments[i]);
    }
    cmdln.argument_count = argument_count-1;
    return cmdln;
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_Flag(MD_CommandLine *cmdln, MD_String8 string)
{
    MD_b32 result = 0;
    for(int i = 0; i < cmdln->argument_count; i += 1)
    {
        if(MD_StringMatch(string, cmdln->arguments[i], 0))
        {
            result = 1;
            cmdln->arguments[i].str = 0;
            cmdln->arguments[i].size = 0;
            break;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_FlagStrings(MD_CommandLine *cmdln, MD_String8 string, int out_count, MD_String8 *out)
{
    MD_b32 result = 0;
    for(int i = 0; i < cmdln->argument_count; i += 1)
    {
        if(MD_StringMatch(string, cmdln->arguments[i], 0))
        {
            cmdln->arguments[i].str = 0;
            cmdln->arguments[i].size = 0;
            if(cmdln->argument_count > i + out_count)
            {
                for(int out_idx = 0; out_idx < out_count; out_idx += 1)
                {
                    out[out_idx] = cmdln->arguments[i+out_idx+1];
                    cmdln->arguments[i+out_idx+1].str = 0;
                    cmdln->arguments[i+out_idx+1].size = 0;
                }
                result = 1;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_FlagIntegers(MD_CommandLine *cmdln, MD_String8 string, int out_count, MD_i64 *out)
{
    MD_b32 result = 0;
    for(int i = 0; i < cmdln->argument_count; i += 1)
    {
        if(MD_StringMatch(string, cmdln->arguments[i], 0))
        {
            cmdln->arguments[i].str = 0;
            cmdln->arguments[i].size = 0;
            if(cmdln->argument_count > i + out_count)
            {
                for(int out_idx = 0; out_idx < out_count; out_idx += 1)
                {
                    out[out_idx] = MD_I64FromString(cmdln->arguments[i+out_idx+1]);
                    cmdln->arguments[i+out_idx+1].str = 0;
                    cmdln->arguments[i+out_idx+1].size = 0;
                }
                result = 1;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_FlagString(MD_CommandLine *cmdln, MD_String8 string, MD_String8 *out)
{
    return MD_CommandLine_FlagStrings(cmdln, string, 1, out);
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_FlagInteger(MD_CommandLine *cmdln, MD_String8 string, MD_i64 *out)
{
    return MD_CommandLine_FlagIntegers(cmdln, string, 1, out);
}

MD_FUNCTION_IMPL MD_b32
MD_CommandLine_Increment(MD_CommandLine *cmdln, MD_String8 **string_ptr)
{
    MD_b32 result = 0;
    MD_String8 *string = *string_ptr;
    if(string == 0)
    {
        for(int i = 0; i < cmdln->argument_count; i += 1)
        {
            if(cmdln->arguments[i].str)
            {
                string = &cmdln->arguments[i];
                break;
            }
        }
    }
    else
    {
        int idx = (int)(string - cmdln->arguments);
        string = 0;
        for(int i = idx+1; i < cmdln->argument_count; i += 1)
        {
            if(cmdln->arguments[i].str)
            {
                string = &cmdln->arguments[i];
                break;
            }
        }
    }
    *string_ptr = string;
    result = !!string;
    return result;
}

MD_FUNCTION_IMPL MD_String8
MD_LoadEntireFile(MD_String8 filename)
{
    MD_String8 file_contents = {0};
    FILE *file = fopen((char*)MD_PushStringCopy(filename).str, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        MD_u64 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_contents.str = _MD_PushArray(_MD_GetCtx(), MD_u8, file_size+1);
        if(file_contents.str)
        {
            file_contents.size = file_size;
            fread(file_contents.str, 1, file_size, file);
        }
        fclose(file);
    }
    return file_contents;
}

MD_FUNCTION_IMPL MD_b32
MD_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info)
{
#if !defined(MD_IMPL_FileIterIncrement)
    return(0);
#else
    return(MD_IMPL_FileIterIncrement(it, path, out_info));
#endif
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
