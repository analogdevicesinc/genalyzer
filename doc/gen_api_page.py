# Generate API page for documentation
import os

def gen_pages():
    header = os.path.join(
        os.path.dirname(__file__), "..", "bindings", "c", "include", "cgenalyzer_simplified_beta.h"
    )

    if not os.path.exists(header):
        print("Header file not found")
        exit()

    with open(header, "r") as f:
        lines = f.readlines()

    functions = []

    for line in lines:
        if "__api" in line:
            line = line.split(" ")
            for l in line:
                if "gn_" in l:
                    # Remove everything after (
                    l = l.split("(")[0]
                    functions.append(l)

            # print(line.strip())
            # functions.append(line.strip())

    count = len(functions)

    # Organize

    set_functions = []
    get_functions = []
    config_functions = []
    generate_functions = []
    operation_functions = []
    missed = []
    for function in functions:
        if "set" in function:
            set_functions.append(function)
            continue
        if "get" in function:
            get_functions.append(function)
            continue
        if "config" in function:
            config_functions.append(function)
            continue
        if "gen" in function:
            generate_functions.append(function)
            continue
        operation_functions.append(function)

    set_count = len(set_functions)
    config_count = len(config_functions)

    print(f"Total functions: {count}")
    print(f"Set functions: {set_count}")
    print(f"Config functions: {config_count}")
    print(f"Get functions: {len(get_functions)}")
    print(f"Generate functions: {len(generate_functions)}")

    # Generate md file for breathe
    page = "<!--- This file is auto-generated. Do not edit -->\n\n"

    page += "# Simplified API Reference\n\n"
    page += "This page contains the (beta) simplified API reference for the Genalyzer library.\n\n"

    # Table of contents

    page += "## Function Groups\n\n"
    page += "- [Set Functions](#set-functions)\n"
    page += "- [Get Functions](#get-functions)\n"
    page += "- [Config Functions](#config-functions)\n"
    page += "- [Generate Functions](#generate-functions)\n"
    page += "- [Operation Functions](#operation-functions)\n\n"

    page += "## Structs\n\n"


    def gen_functions(functions):
        page = ""
        for function in functions:
            page += "```{eval-rst}\n"
            page += f".. doxygenfunction:: {function}\n\n"
            page += "```\n\n"
        return page

    page += "---\n\n"
    page += "## Set Functions\n\n"
    page += gen_functions(set_functions)

    page += "---\n\n"
    page += "## Get Functions\n\n"
    page += gen_functions(get_functions)

    page += "---\n\n"
    page += "## Config Functions\n\n"
    page += gen_functions(config_functions)

    page += "---\n\n"
    page += "## Generate Functions\n\n"
    page += gen_functions(generate_functions)

    page += "---\n\n"
    page += "## Operation Functions\n\n"
    page += gen_functions(operation_functions)

    with open("reference_simplified.md", "w") as f:
        f.write(page)
    print("API page generated")

        