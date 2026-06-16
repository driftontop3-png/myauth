import jwt from "jsonwebtoken";
import bcrypt from "bcryptjs";
import { sql, ensureDb } from "./db.js";

const COOKIE_NAME = "astryn_session";
const TOKEN_TTL = "7d";

function getJwtSecret() {
  return process.env.JWT_SECRET || "astryn-dev-secret-change-in-production";
}

export function setSessionCookie(res, userId) {
  const token = jwt.sign({ userId }, getJwtSecret(), { expiresIn: TOKEN_TTL });
  const secure = process.env.VERCEL === "1" ? "; Secure" : "";
  res.setHeader(
    "Set-Cookie",
    `${COOKIE_NAME}=${token}; Path=/; HttpOnly; SameSite=Lax; Max-Age=604800${secure}`
  );
}

export function clearSessionCookie(res) {
  const secure = process.env.VERCEL === "1" ? "; Secure" : "";
  res.setHeader(
    "Set-Cookie",
    `${COOKIE_NAME}=; Path=/; HttpOnly; SameSite=Lax; Max-Age=0${secure}`
  );
}

export function readSession(req) {
  const cookieHeader = req.headers.cookie || "";
  const match = cookieHeader.match(new RegExp(`${COOKIE_NAME}=([^;]+)`));
  if (!match) return null;

  try {
    return jwt.verify(match[1], getJwtSecret());
  } catch {
    return null;
  }
}

export async function hashPassword(password) {
  return bcrypt.hash(password, 10);
}

export async function verifyPassword(password, hash) {
  return bcrypt.compare(password, hash);
}

export function generateId(length = 12) {
  const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  let result = "";
  for (let i = 0; i < length; i += 1) {
    result += chars[Math.floor(Math.random() * chars.length)];
  }
  return result;
}

export function generateSecret(length = 32) {
  return generateId(length);
}

export function toPublicUser(row) {
  return {
    username: row.username,
    email: row.email,
    appName: row.app_name,
    ownerId: row.owner_id,
    appSecret: row.app_secret,
    appVersion: row.app_version || "1.0",
    createdAt: new Date(row.created_at).getTime(),
  };
}

export function toLicense(row) {
  return {
    id: row.id,
    key: row.license_key,
    status: row.status,
    level: row.level,
    expiryUnit: row.expiry_unit,
    expiryDuration: row.expiry_duration,
    expiryLabel: row.expiry_label,
    note: row.note || "",
    createdAt: new Date(row.created_at).getTime(),
  };
}

export async function getUserById(userId) {
  await ensureDb();
  const { rows } = await sql`SELECT * FROM users WHERE id = ${userId} LIMIT 1`;
  return rows[0] || null;
}

export async function getUserByUsername(username) {
  await ensureDb();
  const { rows } = await sql`SELECT * FROM users WHERE username = ${username} LIMIT 1`;
  return rows[0] || null;
}

export async function getUserByOwnerId(ownerId) {
  await ensureDb();
  const { rows } = await sql`SELECT * FROM users WHERE owner_id = ${ownerId} LIMIT 1`;
  return rows[0] || null;
}

export async function getLicensesForUser(userId) {
  await ensureDb();
  const { rows } = await sql`
    SELECT * FROM licenses
    WHERE user_id = ${userId}
    ORDER BY created_at DESC
  `;
  return rows.map(toLicense);
}

export async function getLicenseStats(userId) {
  await ensureDb();
  const { rows } = await sql`
    SELECT
      COUNT(*)::int AS total,
      COUNT(*) FILTER (WHERE status = 'unused')::int AS unused,
      COUNT(*) FILTER (WHERE status = 'used')::int AS used,
      COUNT(*) FILTER (WHERE status = 'banned')::int AS banned
    FROM licenses
    WHERE user_id = ${userId}
  `;
  return rows[0] || { total: 0, unused: 0, used: 0, banned: 0 };
}

export async function requireUser(req, res) {
  const session = readSession(req);
  if (!session?.userId) {
    res.status(401).json({ success: false, message: "Unauthorized" });
    return null;
  }

  const user = await getUserById(session.userId);
  if (!user) {
    res.status(401).json({ success: false, message: "Unauthorized" });
    return null;
  }

  return user;
}

export function createClientSession(user) {
  return jwt.sign(
    {
      ownerId: user.owner_id,
      appName: user.app_name,
      type: "client",
    },
    getJwtSecret(),
    { expiresIn: "1h" }
  );
}

export function verifyClientSession(sessionId) {
  try {
    const payload = jwt.verify(sessionId, getJwtSecret());
    if (payload.type !== "client") return null;
    return payload;
  } catch {
    return null;
  }
}
