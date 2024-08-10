const std = @import("std");
const net = std.net;
const c = @cImport({
    @cInclude("nhw.h");
    @cInclude("networking/nhwnetworking.h");
});

const ArenaAllocator = std.heap.ArenaAllocator;
var gpa = std.heap.GeneralPurposeAllocator(.{}){};
const allocator = gpa.allocator();

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
pub fn CreateClient(conn: net.Server.Connection) !*Client {
    var client_arena = ArenaAllocator.init(allocator);
    const client = try client_arena.allocator().create(Client);
    client.* = Client.init(client_arena, conn.stream);
    const thread = try std.Thread.spawn(.{}, Client.run, .{client});
    thread.detach();
    return client;
}

pub export fn ConnectClients() ?*void {
    if (listener.accept()) |conn| {
        const client = CreateClient(conn);
        if (client) |clientconnected| {
            c.Log("Established connection");
            return @ptrCast(clientconnected);
        } else |_| {
            c.Log("Failed to launch client thread");
            return null;
        }
    } else |_| {
        c.Log("Failed to establish conntection");
    }
    return null;
}

pub export fn SendMessage(client: *Client, message: [*c]const u8) void {
    client.sendMessage(message);
}

const Client = struct {
    arena: ArenaAllocator,
    stream: net.Stream,

    pub fn init(arena: ArenaAllocator, stream: net.Stream) Client {
        return .{
            .stream = stream,
            .arena = arena,
        };
    }

    fn run(self: *Client) !void {
        const stream = self.stream;
        while (true) {
            var buf: [100]u8 = undefined;
            const n = try stream.read(&buf);
            if (n == 0) {
                return;
            }
            c.MessageCallback(@constCast(&buf[0]), @intCast(n - 2), self);
        }
    }
    fn sendMessage(self: *Client, message: [*c]const u8) void {
        const stream = self.stream;
        const msgzig: []const u8 = std.mem.span(message);
        if (stream.write(msgzig)) |_| {} else |_| {}
    }
};
