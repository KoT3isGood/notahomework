const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const glfw = getGlfw(b, optimize, target);












    const nhw = b.addStaticLibrary(.{
        .name = "nhw",
        .target = target,
        .optimize = optimize,
    });
    nhw.linkLibrary(glfw);
    nhw.linkLibCpp();
    nhw.addIncludePath(.{ .path = "thirdparty/" });
    nhw.addCSourceFiles(.{
        .files = &.{ "src/nhw.cpp", "src/draw/nhwdraw.cpp" },
    });
    b.installArtifact(nhw);

    
}

fn getGlfw(
    b: *std.Build,
    optimize: std.builtin.OptimizeMode,
    target: std.Build.ResolvedTarget,
) *std.Build.Step.Compile {
    const glfw = b.dependency("glfw", .{
        .optimize = optimize,
        .target = target,
    });

    return glfw.artifact("glfw");
}