import {compilers} from "../../TS/mmake/src/ts/core/Compiler";
import {UserConfig} from "../../TS/mmake/src/ts/core/Config";
import {UserTarget} from "../../TS/mmake/src/ts/core/Target";
import {ProductionModes} from "../../TS/mmake/src/ts/core/ProductionModes";

const shared: Partial<UserTarget> = {
    libraries: [
        {binary: "stdc++fs"},
        {include: "../../C++/json/single_include"},
        {include: "`llvm-config --includedir`"},
    ],
    filter: path => !(path.directory && path.directory.fileName && path.directory.fileName.raw === "lzma"),
    flags: ProductionModes.share(["pthread"]),
};

export const mmake: UserConfig = {
    name: "SiliconScratch",
    targets: [
        {
            ...shared,
            target: "native",
            compiler: compilers.gcc,
        },
        {
            ...shared,
            target: "wasm",
            compiler: compilers.emscripten,
        },
    ],
};