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
    nhw.addLibraryPath(.{ .path = "thirdparty/" });
    const vulkanName = if(target.result.os.tag == .windows) "vulkan-1" else "vulkan";
    const openalName = if(target.result.os.tag == .windows) "OpenAL32" else "OpenAL";
    nhw.linkSystemLibrary(vulkanName);
    nhw.linkSystemLibrary(openalName);
    nhw.addCSourceFiles(.{
        .files = &.{ "src/nhw.cpp", "src/draw/nhwdraw.cpp","src/draw/nhwtexture.cpp","src/device/nhwdevice.cpp","src/audio/nhwaudio.cpp","thirdparty/stb_vorbis.c" },
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