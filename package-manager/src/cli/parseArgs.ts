type ParsedArgs = {
    flags: Record<string, string | boolean>;
    positional: string[];
}