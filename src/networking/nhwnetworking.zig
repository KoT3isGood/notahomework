const std = @import("std");
const net = std.net;
const c = @cImport({
    @cInclude("nhw.h");
});

var addr: std.net.Address = undefined;
var listener: net.Server = undefined;

pub fn CreateListener(port: u16) !void {
    addr = std.net.Address.initIp4(.{ 127, 0, 0, 1 }, port);
    listener = try addr.listen(.{
        .reuse_address = true,
        .kernel_backlog = 1024,
    });
    c.Log("Listening at 127.0.0.1:%i", port);
}

pub export fn CreateServer(port: u16) void {
    CreateListener(port) catch {};
}

pub export fn ConnectClients() ?*void {
    if (listener.accept()) |_| {
        c.Log("Established connection");
    } else |_| {
        c.Log("Failed to establish conntection");
    }
    return null;
}
