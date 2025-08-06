"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const cmdList_1 = require("./utils/cmdList");
const parseArgs_1 = require("./utils/parseArgs");
const package_json_1 = __importDefault(require("../package.json"));
const defaultHelp_1 = __importDefault(require("./utils/defaultHelp"));
class UDT {
    constructor({ argv = [] } = {}) {
        this.argv = [];
        this.argv = argv;
    }
    static async cmd(cmd) {
        const c = (0, cmdList_1.cmdRef)(cmd);
        if (!c) {
            console.error(`Unknown command '${cmd}'`);
            return null;
        }
        try {
            return await Promise.resolve(`${`./commands/${c}.js`}`).then(s => __importStar(require(s)));
        }
        catch {
            console.error(`Failed to load command module: './commands/${c}.js'`);
            return null;
        }
    }
    async execute(cmd, args = this.argv) {
        try {
            const cmdModule = await this.constructor.cmd(cmd);
            if (!cmdModule.default) {
                console.error(`Command '${cmd}' not found.`);
                return;
            }
            const Command = cmdModule.default;
            const command = new Command(this);
            if (command.customParams) {
            }
            else {
                const parsed = (0, parseArgs_1.parseCmdArgs)(args, Command.params);
                if (!parsed)
                    return;
                if (parsed.$global.help) {
                    await this.execute('help', [(0, cmdList_1.cmdRef)(cmd) ?? '']);
                    return;
                }
                await command.execute(parsed.$global, parsed.$command, parsed._);
            }
        }
        catch (err) {
            console.error(err instanceof Error ? err.message : String(err));
        }
    }
    static get version() {
        return package_json_1.default.version;
    }
    static get usage() {
        return (0, defaultHelp_1.default)(new this);
    }
}
exports.default = UDT;
