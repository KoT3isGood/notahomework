pub const packages = struct {
    pub const @"122010c1a745ea06dee3012fbd3b311bd3d75ec39ded6bf566b36ebe3cd8da482347" = struct {
        pub const build_root = "C:\\Users\\brate\\AppData\\Local\\zig\\p\\122010c1a745ea06dee3012fbd3b311bd3d75ec39ded6bf566b36ebe3cd8da482347";
        pub const build_zig = @import("122010c1a745ea06dee3012fbd3b311bd3d75ec39ded6bf566b36ebe3cd8da482347");
        pub const deps: []const struct { []const u8, []const u8 } = &.{};
    };
    pub const @"122017098e4ca00dac1a9d30d152a6e6e9522843219c6df4489210fb7c8a6e4c7c1a" = struct {
        pub const available = true;
        pub const build_root = "C:\\Users\\brate\\AppData\\Local\\zig\\p\\122017098e4ca00dac1a9d30d152a6e6e9522843219c6df4489210fb7c8a6e4c7c1a";
        pub const build_zig = @import("122017098e4ca00dac1a9d30d152a6e6e9522843219c6df4489210fb7c8a6e4c7c1a");
        pub const deps: []const struct { []const u8, []const u8 } = &.{};
    };
    pub const @"12202b5b936e38c0ebf384de6b3c8e7771b1f17ba952c3fc8e3a0c852fc4710b537a" = struct {
        pub const build_root = "C:\\Users\\brate\\AppData\\Local\\zig\\p\\12202b5b936e38c0ebf384de6b3c8e7771b1f17ba952c3fc8e3a0c852fc4710b537a";
        pub const build_zig = @import("12202b5b936e38c0ebf384de6b3c8e7771b1f17ba952c3fc8e3a0c852fc4710b537a");
        pub const deps: []const struct { []const u8, []const u8 } = &.{
            .{ "xcode_frameworks", "122010c1a745ea06dee3012fbd3b311bd3d75ec39ded6bf566b36ebe3cd8da482347" },
            .{ "vulkan_headers", "122017098e4ca00dac1a9d30d152a6e6e9522843219c6df4489210fb7c8a6e4c7c1a" },
            .{ "wayland_headers", "12207decf58bee217ae9c5340a6852a62e7f5af9901bef9b1468d93e480798898285" },
            .{ "x11_headers", "1220ce35d8f1556afd5bf4796a7899d459f9c628b989f247eaf6aa00fbad10a88c9f" },
        };
    };
    pub const @"12207decf58bee217ae9c5340a6852a62e7f5af9901bef9b1468d93e480798898285" = struct {
        pub const available = false;
    };
    pub const @"1220ce35d8f1556afd5bf4796a7899d459f9c628b989f247eaf6aa00fbad10a88c9f" = struct {
        pub const available = false;
    };
};

pub const root_deps: []const struct { []const u8, []const u8 } = &.{
    .{ "glfw", "12202b5b936e38c0ebf384de6b3c8e7771b1f17ba952c3fc8e3a0c852fc4710b537a" },
};
