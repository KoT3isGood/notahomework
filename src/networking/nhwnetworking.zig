const std = @import("std");
const net = std.net;
const c = @cImport({
    @cDefine("NHW_ZIG", "");
    @cInclude("stdbool.h");
    @cInclude("nhw.h");
});

var addr: std.net.Address = undefined;
var listener: net.Address.ListenError!net.Server = undefined;
pub export fn CreateServer(port: u16) void {
    addr = std.net.Address.initIp4(.{ 127, 0, 0, 1 }, port);
    listener = addr.listen(.{
        .reuse_address = true,
        .kernel_backlog = 1024,
    });
    c.Log("Listening at 127.0.0.1:%i", port);
}
