#include "md.h"
#include "md.c"

typedef struct TestStruct
{
    char (*test)[256];
} TestStruct;

int
main(int argc, char** argv)
{
    MD_Node *first = MD_NilNode();
    MD_Node *last  = MD_NilNode();

    fprintf(stderr, "Running metaprogram.\n");

    MD_FileInfo file_info;
    for (MD_FileIter it = {}; MD_FileIterIncrement(&it, MD_S8Lit("*.mc"), &file_info);)
    {
        if (file_info.filename.size)
        {
            MD_Node *root = MD_ParseWholeFile(file_info.filename);
            MD_PushSibling(&first, &last, MD_NilNode(), root);
        }
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
                if(MD_NodeHasTag(node, MD_S8Lit("struct")))
                {
                    MD_OutputTree_C_Struct(out_file, node);
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
