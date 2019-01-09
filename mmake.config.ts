import {compilers} from "../../mmake/src/ts/core/Compiler";
import {UserConfig} from "../../mmake/src/ts/core/Config";

export const mmake: UserConfig = {
    name: "ScratchWasmRenderer",
    targets: [
        {
            target: "native",
            compiler: compilers.gcc,
        },
        {
            target: "wasm",
            compiler: compilers.emscripten,
        },
    ],
};