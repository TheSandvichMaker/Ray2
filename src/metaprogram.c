#include "md.h"
#include "md.c"

int
main(int argc, char** argv)
{
    fprintf(stderr, "Running metaprogram.\n");

    MD_Node *first = MD_NilNode();
    MD_Node *last  = MD_NilNode();

    MD_FileInfo file_info;
    for (MD_FileIter it = { 0 }; MD_FileIterIncrement(&it, MD_S8Lit("*.mc"), &file_info);)
    {
        MD_Node *root = MD_ParseWholeFile(file_info.filename);
        MD_PushSibling(&first, &last, MD_NilNode(), root);
    }

    if (first != MD_NilNode())
    {
        fprintf(stderr, "Outputting C code.\n");
    }

    for (MD_EachNode(root, first))
    {
        MD_String8 trimmed_filename = MD_TrimExtension(root->filename);
        MD_String8 out_filename = MD_PushStringF("generated/%.*s.h", MD_StringExpand(trimmed_filename));

        fprintf(stderr, "Generated code: %.*s -> %.*s\n",
                MD_StringExpand(root->filename), MD_StringExpand(out_filename));

        FILE *out_file = fopen((char *)out_filename.str, "wb");
        if (out_file)
        {
            for (MD_EachNode(node, root->first_child))
            {
                if (MD_NodeHasTag(node, MD_S8Lit("struct")))
                {
                    MD_OutputTree_C_Struct(out_file, node);
                }
                else if (MD_NodeHasTag(node, MD_S8Lit("enum")))
                {
                    fprintf(out_file, "typedef enum test_enum;\n");
                    fprintf(out_file, "enum test_enum\n{\n");
                    for (MD_EachNode(enumeration, node->first_child))
                    {
                        fprintf(out_file, "    %.*s", MD_StringExpand(enumeration->string));
                        if (enumeration->first_child)
                        {
                            MD_Expr *expr = MD_ParseAsExpr(enumeration->first_child, enumeration->last_child);
                            MD_i64 value = MD_EvaluateExpr_I64(expr);
                            fprintf(out_file, " = %lld", value);
                        }
                        fprintf(out_file, ",\n");
                    }
                    fprintf(out_file, "};\n\n");

                    if (MD_NodeHasTag(node, MD_S8Lit("namefunc")))
                    {
                        fprintf(out_file, "static const char *\n%.*s_ToString(%.*s Value)\n{\n",
                                MD_StringExpand(node->string),
                                MD_StringExpand(node->string));
                        fprintf(out_file, "    switch (Value)\n    {\n");
                        for (MD_EachNode(enumeration, node->first_child))
                        {
                            fprintf(out_file, "        case %.*s: return \"%.*s\";\n",
                                    MD_StringExpand(enumeration->string),
                                    MD_StringExpand(enumeration->string));
                        }
                        fprintf(out_file, "    }\n");
                        fprintf(out_file, "    return \"Unknown %.*s Value\";\n",
                                MD_StringExpand(node->string));
                        fprintf(out_file, "}\n\n");
                    }

                    if (MD_NodeHasTag(node, MD_S8Lit("nametable")))
                    {
                        fprintf(out_file, "const char *%.*s_Names[] =\n{\n",
                                MD_StringExpand(node->string));
                        for (MD_EachNode(enumeration, node->first_child))
                        {
                            fprintf(out_file, "    [%.*s] = \"%.*s\",\n",
                                    MD_StringExpand(enumeration->string),
                                    MD_StringExpand(enumeration->string));
                        }
                        fprintf(out_file, "}\n\n");
                    }
                }
            }
        }
        else
        {
            fprintf(stderr, "METAPROGRAM ERROR: Could not open '%.*s' for writing.\n",
                    MD_StringExpand(out_filename));
        }
    }
}
