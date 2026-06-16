import { sql, ensureDb } from "../../lib/db.js";
import {
  hashPassword,
  setSessionCookie,
  toPublicUser,
  generateId,
  generateSecret,
  getLicensesForUser,
  getLicenseStats,
} from "../../lib/auth.js";
import { parseBody, sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    await ensureDb();
    const body = await parseBody(req);
    const username = String(body.username || "").trim();
    const email = String(body.email || "").trim();
    const password = String(body.password || "");
    const appName = String(body.appName || "").trim();

    if (username.length < 3) {
      return sendJson(res, 400, { success: false, message: "Username must be at least 3 characters." });
    }
    if (password.length < 6) {
      return sendJson(res, 400, { success: false, message: "Password must be at least 6 characters." });
    }
    if (!email || !appName) {
      return sendJson(res, 400, { success: false, message: "Email and application name are required." });
    }

    const existing = await sql`
      SELECT id FROM users WHERE LOWER(username) = LOWER(${username}) LIMIT 1
    `;
    if (existing.rows.length) {
      return sendJson(res, 409, { success: false, message: "Username already exists." });
    }

    const passwordHash = await hashPassword(password);
    const ownerId = generateId(10);
    const appSecret = generateSecret(32);

    const inserted = await sql`
      INSERT INTO users (username, email, password_hash, app_name, owner_id, app_secret)
      VALUES (${username}, ${email}, ${passwordHash}, ${appName}, ${ownerId}, ${appSecret})
      RETURNING *
    `;

    const user = inserted.rows[0];
    setSessionCookie(res, user.id);

    const licenses = await getLicensesForUser(user.id);
    const stats = await getLicenseStats(user.id);

    sendJson(res, 201, {
      success: true,
      message: "Account created successfully.",
      user: toPublicUser(user),
      licenses,
      stats,
    });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "Registration failed." });
  }
}
